#ifndef CORAL_UTIL_UNIFORMBLOCKBUILDER_HPP
#define CORAL_UTIL_UNIFORMBLOCKBUILDER_HPP

#include <Coral/ShaderModuleReflection.h>

#include <vector>
#include <span>
#include <string_view>
#include <string>
#include <unordered_map>
#include <variant>
#include <cassert>

namespace Coral
{

/*!
 * 
 */
template<typename Vec2F, typename Vec3F, typename Vec4F, typename Vec2I, typename Vec3I, typename Vec4I, typename Mat33F, typename Mat44F>
class UniformBlockBuilder
{
public:

    UniformBlockBuilder(const CoBufferInfo& info)
    {
        // Allocate the data buffer to contain the entire uniform block
        mData.resize(info.structure.size, std::byte(0));

        // Build a lookup table to easily find the start of each member in the uniform block
        buildOffsetTable(info);
    }

    UniformBlockBuilder() = default;

    /*!
     * \brief Set the value at the given index 
     * Fails if the format of the member at the given index does not match the type.
     */
    template<typename T>
    bool set(size_t index, const T& value)
    {
        return setValue(index, getTypeInfo<T>(), reinterpret_cast<const std::byte*>(&value));
    }
 
    /*!
     * \brief Set the value at the given index
     * Fails if the format of the member at the given index does not match the type.
     */
    template<typename T>
    bool set(std::string_view name, const T& value)
    {
        return setValue(name, getTypeInfo<T>(), reinterpret_cast<const std::byte*>(&value));
    }

    /*!
     * Get the aligned uniform block data ready for uploading to the GPU
     */
    const std::span<const std::byte> data() const
    {
        return mData;
    }

private:

    void buildOffsetTableRecursive(const CoStructureInfo& structure, const std::string& parentName)
    {
        for (const auto& member : std::span(structure.pMembers, structure.memberCount))
        {
            TypeInfo typeInfo;
            uint32_t size{ 0 };
            switch (member.type)
            {
                case CO_STRUCTURE_MEMBER_TYPE_STRUCTURE:
                    buildOffsetTableRecursive(member.structure, parentName + "." + member.pName);
                    continue;
                case CO_STRUCTURE_MEMBER_TYPE_SCALAR:
                    typeInfo = member.scalar;
                    size     = coScalarTypeGetSizeInBytes(member.scalar.dataType);
                    break;
                case CO_STRUCTURE_MEMBER_TYPE_VECTOR:
                    typeInfo = member.vector;
                    size     = member.vector.components * coScalarTypeGetSizeInBytes(member.vector.dataType);
                    break;
                case CO_STRUCTURE_MEMBER_TYPE_MATRIX:
                    typeInfo = member.matrix;
                    size     = member.matrix.rows * member.matrix.columns * coScalarTypeGetSizeInBytes(member.matrix.dataType);
                    break;
            }

            std::string fullName = parentName.empty() ? member.pName : parentName + "." + member.pName;

            mMembers.push_back({
                .typeInfo = typeInfo,
                .name     = fullName,
                .offset   = member.offset,
                .size     = size
            });

            mNameToIndexLookUp[fullName] = mMembers.size() - 1;
        }
    }

    void buildOffsetTable(const CoBufferInfo& bufferInfo)
    {
        buildOffsetTableRecursive(bufferInfo.structure, "");
    }

    using TypeInfo = std::variant<CoScalarInfo, CoVectorInfo, CoMatrixInfo>;

    template<typename T>
    constexpr static TypeInfo getTypeInfo();

    template<>
    constexpr static TypeInfo getTypeInfo<float>()
    {
        return CoScalarInfo{ .dataType = CO_SCALAR_TYPE_FLOAT32 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<int>()
    {
        return CoScalarInfo{ .dataType = CO_SCALAR_TYPE_INT32 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<uint32_t>()
    {
        return CoScalarInfo{ .dataType = CO_SCALAR_TYPE_UINT32 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<uint64_t>()
    {
        return CoScalarInfo{ .dataType = CO_SCALAR_TYPE_UINT64 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<int64_t>()
    {
        return CoScalarInfo{ .dataType = CO_SCALAR_TYPE_INT64 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Vec2F>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_FLOAT32, .components = 2 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Vec3F>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_FLOAT32, .components = 3 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Vec4F>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_FLOAT32, .components = 4 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Vec2I>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_INT32, .components = 2 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Vec3I>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_INT32, .components = 3 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Vec4I>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_INT32, .components = 4 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Mat33F>()
    {
        return CoMatrixInfo{ .dataType = CO_SCALAR_TYPE_FLOAT32, .rows = 3, .columns = 3 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Mat44F>()
    {
        return CoMatrixInfo{ .dataType = CO_SCALAR_TYPE_FLOAT32, .rows = 4, .columns = 4 };
    }

    bool setValue(std::string_view name, const TypeInfo& info, const std::byte* value)
    {
        auto iter = mNameToIndexLookUp.find(std::string(name));
        if (iter == mNameToIndexLookUp.end())
        {
            return false;
        }

        return setValue(iter->second, info, value);
    }

    bool setValue(size_t index, const TypeInfo& info, const std::byte* value)
    {
        if (mMembers.size() <= index)
        {
            return false;
        }

        return setValueUnchecked(index, value);
    }

    bool setValueUnchecked(size_t index, const std::byte* value)
    {
        auto& member = mMembers[index];

        auto data = mData.data() + member.offset;

        if (auto matrix = std::get_if<CoMatrixInfo>(&member.typeInfo))
        {
            auto colSize = sizeof(float) * matrix->rows;
            for (uint32_t i = 0; i < matrix->columns; ++i)
            {
                std::memcpy(data, value, colSize);
                value += colSize;
                data += matrix->stride;
            }
        }
        else
        {
            std::memcpy(data, value, member.size);
        }

        return true;
    }

    struct BlockMember
    {
        TypeInfo typeInfo;
        std::string name;
        uint32_t offset{ 0 };
        uint32_t size{ 0 };
    };

    std::vector<BlockMember> mMembers;
    std::unordered_map<std::string, uint32_t> mNameToIndexLookUp;
    std::vector<std::byte> mData;

}; // class UniformBlockBuilder

} // namespace Coral

#endif // !CORAL_UTIL_UNIFORMBLOCKBUILDER_HPP

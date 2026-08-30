#ifndef CORAL_UTIL_UNIFORMBLOCKBUILDER_HPP
#define CORAL_UTIL_UNIFORMBLOCKBUILDER_HPP

#include <Coral/ShaderModule.h>

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
 */
template<typename Vec2F, typename Vec3F, typename Vec4F, typename Vec2I, typename Vec3I, typename Vec4I, typename Mat33F, typename Mat44F>
class UniformBlockBuilder
{
public:

    UniformBlockBuilder(const CoBufferInfo& info)
        : mMembers(buildMemberInfos(info))
    {
        if (mMembers.empty())
        {
            return;
        }

        assert(info.structure.count > 0);

        auto bufferSize = info.structure.stride * info.structure.count;
        mData.resize(bufferSize, std::byte(0));

        for (size_t i = 0; i < mMembers.size(); ++i)
        {
            mNameToIndexLookUp[mMembers[i].name] = i;
        }
    }

    UniformBlockBuilder() = default;

    /*!
     * \brief Set the value at the given index 
     * Fails if the format of the member at the given index does not match the type.
     */
    template<typename T>
    bool set(size_t index, const T& value, uint32_t element = 0)
    {
        return setValue(index, getTypeInfo<T>(), reinterpret_cast<const std::byte*>(&value), element);
    }
 
    /*!
     * \brief Set the value at the given index
     * Fails if the format of the member at the given index does not match the type.
     */
    template<typename T>
    bool set(std::string_view name, const T& value, uint32_t element = 0)
    {
        return setValue(name, getTypeInfo<T>(), reinterpret_cast<const std::byte*>(&value), element);
    }

    /*!
     * Get the aligned uniform block data ready for uploading to the GPU
     */
    const std::span<const std::byte> data() const
    {
        return mData;
    }

private:

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
    constexpr static TypeInfo getTypeInfo<Vec2F>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_FLOAT32, .componentCount = 2 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Vec3F>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_FLOAT32, .componentCount = 3 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Vec4F>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_FLOAT32, .componentCount = 4 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Vec2I>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_INT32, .componentCount = 2 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Vec3I>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_INT32, .componentCount = 3 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Vec4I>()
    {
        return CoVectorInfo{ .dataType = CO_SCALAR_TYPE_INT32, .componentCount = 4 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Mat33F>()
    {
        return CoMatrixInfo{ .dataType = CO_SCALAR_TYPE_INT32, .rowCount = 3, .columnCount = 3 };
    }

    template<>
    constexpr static TypeInfo getTypeInfo<Mat44F>()
    {
        return CoMatrixInfo{ .dataType = CO_SCALAR_TYPE_INT32, .rowCount = 4, .columnCount = 4 };
    }

    struct MemberInfo
    {
        // Name of the member
        std::string name;
        // Type of the member
        TypeInfo type;
        // Number of elements if the member is an array
        uint32_t count;
        // Distance in bytes between two consecutive array elements in the member
        uint32_t stride;
        // Offset to the start of the member in bytes
        size_t offset{ 0 };
    };

    bool setValue(std::string_view name, const TypeInfo& info, const std::byte* value, uint32_t element)
    {
        auto iter = mNameToIndexLookUp.find(std::string(name));
        if (iter == mNameToIndexLookUp.end())
        {
            return false;
        }

        return setValue(iter->second, info, value, element);
    }

    bool setValue(size_t index, const TypeInfo& info, const std::byte* value, uint32_t element)
    {
        if (mMembers.size() <= index ||
            //mMembers[index].type != info ||
            mMembers[index].count <= element)
        {
            return false;
        }

        return setValueUnchecked(index, value, element);
    }

    bool setValueUnchecked(size_t index, const std::byte* value, uint32_t element)
    {
        auto& member = mMembers[index];

        auto data = mData.data() + member.offset + element * member.stride;

        if (auto matrix = std::get_if<CoMatrixInfo>(&member.type))
        {
            auto colSize = sizeof(float) * matrix->rowCount;
            for (uint32_t i = 0; i < matrix->columnCount; ++i)
            {
                std::memcpy(data, value, colSize);
                value += colSize;
                data += matrix->stride;
            }
        }
        else
        {
            std::memcpy(data, value, member.stride);
        }
        
        return true;
    }

    static inline void
    addMember(const CoStructMemberInfo& member, const std::string& prefix, std::vector<MemberInfo>& result)
    {
        if (member.type == CO_STRUCT_MEMBER_TYPE_STRUCT)
        {
            for (size_t i = 0; i < member.structure.memberCount; ++i)
            {
                const auto& subMember = member.structure.pMembers[i];
                addMember(subMember, prefix + member.pName + ".", result);
            }
        }
        else
        {
            result.push_back({
                .name = prefix + member.pName,
                .type = member.type == CO_STRUCT_MEMBER_TYPE_SCALAR ? TypeInfo(member.scalar) :
                        member.type == CO_STRUCT_MEMBER_TYPE_VECTOR ? TypeInfo(member.vector) :
                                                                      TypeInfo(member.matrix),
                .count = member.count,
                .stride = member.stride,
                .offset = member.offset });
        }
    }

    static inline std::vector<MemberInfo>
    buildMemberInfos(const CoBufferInfo& bufferInfo)
    {
        std::vector<MemberInfo> result;

        addMember(bufferInfo.structure, "", result);

        return result;
    }

    std::vector<MemberInfo> mMembers;

    std::unordered_map<std::string_view, uint32_t> mNameToIndexLookUp;

    std::vector<std::byte> mData;

}; // class UniformBlockBuilder

} // Coral

#endif // !CORAL_UTIL_UNIFORMBLOCKBUILDER_HPP

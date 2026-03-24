#ifndef CORAL_UNIFORMBLOCKBUILDER_HPP
#define CORAL_UNIFORMBLOCKBUILDER_HPP

#include <CoralUtil/System.hpp>

#include <Coral/ShaderModule.h>

#include <vector>
#include <span>
#include <string_view>
#include <string>

#include <unordered_map>

#include <cassert>

template<CoUniformBufferLayout L>
class Layout
{
};

// Traits for std140 memory layout
/**
 * std140 layout specs: https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf#page=159
 * 
 * 1. If the member is a scalar consuming N basic machine units, the base alignment is N.
 * 2. If the member is a two- or four-component vector with components consuming N basic machine units, the base 
 *    alignment is 2N or 4N, respectively
 * 3. If the member is a three-component vector with components consuming N basic machine units, the base alignment
 *    is 4N.
 * 4. If the member is an array of scalars or vectors, the base alignment and array stride are set to match the
 *    base alignment of a single array element, according to rules (1), (2), and (3), and rounded up to the base
 *    alignment of a vec4. The array may have padding at the end; the base offset of the member following the array
 *    is rounded up to the next multiple of the base alignment.
 * 5. If the member is a column-major matrix with C columns and R rows, the matrix is stored identically to an
 *    array of C column vectors with R components each, according to rule (4).
 * 6. If the member is an array of S column - major matrices with C columns and R rows, the matrix is stored
 *    identically to a row of S × C column vectors with R components each, according to rule(4).
 */
template<>
class Layout<CO_UNIFORM_BUFFER_LAYOUT_STD_140>
{
public:
    // Get the memory alignment for the given format
    static uint32_t alignmentInBytes(CoUniformFormat format)
    {
        // We consume 4 machine units (float32, int32)
        constexpr uint32_t N = 4;

        switch (format)
        {
            case CO_UNIFORM_FORMAT_BOOL:    return N;
            case CO_UNIFORM_FORMAT_INT32:    return N;
            case CO_UNIFORM_FORMAT_FLOAT:    return N;
            case CO_UNIFORM_FORMAT_VEC2I:    return N * 2;
            case CO_UNIFORM_FORMAT_VEC2F:    return N * 2;
            case CO_UNIFORM_FORMAT_VEC3I:    return N * 4;
            case CO_UNIFORM_FORMAT_VEC3F:    return N * 4;
            case CO_UNIFORM_FORMAT_VEC4I:    return N * 4;
            case CO_UNIFORM_FORMAT_VEC4F:    return N * 4;
            case CO_UNIFORM_FORMAT_MAT33F:    return N * 4;
            case CO_UNIFORM_FORMAT_MAT44F:    return N * 4;
        }
        assert(false);
        return 0;
    }

    // Get the stride between elements in an array with the given format
    static uint32_t strideInBytes(CoUniformFormat format)
    {
        // We consume 4 machine units (float32, int32)
        constexpr uint32_t N = 4;

        switch (format)
        {
            case CO_UNIFORM_FORMAT_BOOL:    return N;
            case CO_UNIFORM_FORMAT_INT32:    return N;
            case CO_UNIFORM_FORMAT_FLOAT:    return N;
            case CO_UNIFORM_FORMAT_VEC2I:    return N * 2;
            case CO_UNIFORM_FORMAT_VEC2F:    return N * 2;
            case CO_UNIFORM_FORMAT_VEC3I:    return N * 4;
            case CO_UNIFORM_FORMAT_VEC3F:    return N * 4;
            case CO_UNIFORM_FORMAT_VEC4I:    return N * 4;
            case CO_UNIFORM_FORMAT_VEC4F:    return N * 4;
            case CO_UNIFORM_FORMAT_MAT33F:    return N * 12;
            case CO_UNIFORM_FORMAT_MAT44F:    return N * 16;
        }

        assert(false);
        return 0;
    }
};


constexpr uint32_t
nextMultipleOf(uint32_t n, uint32_t v)
{
    return ((v + n - 1) / n) * n;
}


struct MemberLayout
{
    // Name of the member
    std::string name;
    // Format of the member
    CoUniformFormat format;
    // Number of elements if the member is an array
    uint32_t count;
    // Distance in bytes between two consecutive array elements in the member
    uint32_t stride;
    // Offset to the start of the member in bytes
    size_t offset{ 0 };
};


std::vector<MemberLayout>
getMemberLayouts(const CoUniformBlockDefinition& definition)
{
    std::vector<MemberLayout> result;

    size_t offset{ 0 };
    for (size_t i = 0; i < definition.memberCount; ++i)
    {
        const auto& member = definition.pMembers[i];

        auto alignment = Layout<CO_UNIFORM_BUFFER_LAYOUT_STD_140>::alignmentInBytes(member.type);
        auto stride    = Layout<CO_UNIFORM_BUFFER_LAYOUT_STD_140>::strideInBytes(member.type);

        offset = nextMultipleOf(alignment, offset);

        result.push_back({ member.pName, member.type, member.count, stride, offset });

        offset += stride * member.count;
    }
    
    return result;
}


namespace Coral
{

template<typename Vec2F, typename Vec3F, typename Vec4F, typename Vec2I, typename Vec3I, typename Vec4I, typename Mat33F, typename Mat44F>
class CORALUTIL_API UniformBlockBuilder
{
public:

    UniformBlockBuilder(const CoUniformBlockDefinition& definition)
        : mMembers(getMemberLayouts(definition))
    {
        uint32_t bufferSize{ 0 };

        if (mMembers.empty())
        {
            return;
        }

        auto& last = mMembers.back();
        bufferSize = last.offset + last.stride * last.count;

        mData.resize(bufferSize, std::byte(0));

        for (size_t i = 0; i < mMembers.size(); ++i)
        {
            mNameToIndexLookUp[mMembers[i].name] = i;
        }
    }

    template<typename T>
    constexpr static CoUniformFormat getUniformFormat()
    {
        if constexpr (std::same_as<T, float>)
        {
            return CO_UNIFORM_FORMAT_FLOAT;
        }

        if constexpr (std::same_as<T, Vec2F>)
        {
            return CO_UNIFORM_FORMAT_VEC2F;
        }

        if constexpr (std::same_as<T, Vec3F>)
        {
            return CO_UNIFORM_FORMAT_VEC3F;
        }

        if constexpr (std::same_as<T, Vec4F>)
        {
            return CO_UNIFORM_FORMAT_VEC4F;
        }

        if constexpr (std::same_as<T, int>)
        {
            return CO_UNIFORM_FORMAT_INT32;
        }

        if constexpr (std::same_as<T, Vec2I>)
        {
            return CO_UNIFORM_FORMAT_VEC2I;
        }

        if constexpr (std::same_as<T, Vec3I>)
        {
            return CO_UNIFORM_FORMAT_VEC3I;
        }

        if constexpr (std::same_as<T, Vec4I>)
        {
            return CO_UNIFORM_FORMAT_VEC4I;
        }

        if constexpr (std::same_as<T, Mat33F>)
        {
            return CO_UNIFORM_FORMAT_MAT33F;
        }

        if constexpr (std::same_as<T, Mat44F>)
        {
            return CO_UNIFORM_FORMAT_MAT44F;
        }

        if constexpr (std::same_as<T, bool>)
        {
            return CO_UNIFORM_FORMAT_BOOL;
        }
    }

    UniformBlockBuilder() = default;

    //UniformBlockBuilder(const CoUniformBufferDefinition& definition);

    /// Set the value at the given index
    /*
     * Fails if the format of the member at the given index does not match the type.
     */
    template<typename T>
    bool set(size_t index, const T& value, uint32_t element = 0)
    {
        return setValue(index, getUniformFormat<T>(), reinterpret_cast<const std::byte*>(&value), element);
    }

    /// Set the value at the given index
    /*
     * Fails if the format of the member at the given index does not match the type.
     */
    template<typename T>
    bool set(std::string_view name, const T& value, uint32_t element = 0)
    {
        return setValue(name, getUniformFormat<T>(), reinterpret_cast<const std::byte*>(&value), element);
    }

    /// Set the int value with the given name
    /*
     * Fails if the format of the member at the given index is not CO_UNIFORM_FORMAT_INT32.
     */
    bool set(size_t index, int value, uint32_t element = 0)
    {
        return setValue(index, CO_UNIFORM_FORMAT_INT32, reinterpret_cast<const std::byte*>(&value), element);
    }

    /// Get the aligned uniform block data
    const std::span<const std::byte> data() const
    {
        return mData;
    }

private:

    bool setValue(std::string_view name, CoUniformFormat type, const std::byte* value, uint32_t element)
    {
        auto iter = mNameToIndexLookUp.find(std::string(name));
        if (iter == mNameToIndexLookUp.end())
        {
            return false;
        }

        return setValue(iter->second, type, value, element);
    }

    bool setValue(size_t index, CoUniformFormat format, const std::byte* value, uint32_t element)
    {
        if (mMembers.size() <= index ||
            mMembers[index].format != format ||
            mMembers[index].count <= element)
        {
            return false;
        }

        return setValueUnchecked(index, format, value, element);
    }

    bool setValueUnchecked(size_t index, CoUniformFormat format, const std::byte* value, uint32_t element)
    {
        auto& member = mMembers[index];

        auto data = mData.data() + member.offset + element * member.stride;

        if (format == CO_UNIFORM_FORMAT_MAT33F)
        {
            auto* m = reinterpret_cast<const float*>(value);

            std::array<float, 12> padded =
            {
                m[0], m[1], m[2], 0.f,
                m[3], m[4], m[5], 0.f,
                m[6], m[7], m[8], 0.f,
            };

            std::memcpy(data, padded.data(), member.stride);

            return true;
        }
        else
        {
            std::memcpy(data, value, member.stride);
        }

        return true;
    }

    std::vector<MemberLayout> mMembers;

    std::unordered_map<std::string_view, uint32_t> mNameToIndexLookUp;

    std::vector<std::byte> mData;
};

} // Coral


#endif // !CORAL_UNIFORMBLOCKBUILDER_HPP

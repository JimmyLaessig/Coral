#ifndef CORAL_UNIFORMBLOCKBUILDER_HPP
#define CORAL_UNIFORMBLOCKBUILDER_HPP

#include <Coral/System.hpp>

#include <Coral/ShaderModule.hpp>

#include <vector>
#include <span>
#include <string_view>


namespace Coral
{

class CORAL_API UniformBlockBuilder
{
public:

	UniformBlockBuilder() = default;

	UniformBlockBuilder(const UniformBlockDefinition& definition);

	/// Set the bool value at the given index
	/*
	 * Fails if the format of the member at the given index is not Coral::UniformFormat::BOOL.
	 */
	bool setScalar(size_t index, bool value, uint32_t element = 0);

	/// Set the int value at the given index
	/*
	 * Fails if the format of the member at the given index is not Coral::UniformFormat::INT32.
	 */
	bool setScalar(size_t index, int value, uint32_t element = 0);

	/// Set the float value at the given index
	/*
	 * Fails if the format of the member at the given index is not Coral::UniformFormat::FLOAT.
	 */
	bool setScalar(size_t index, float value, uint32_t element = 0);

	/// Set the bool value with the given name
	/*
	 * Fails if the format of the member with the given name is not CoralCoral::UniformFormat::BOOL.
	 */
	bool setScalar(std::string_view name, bool value, uint32_t element = 0);

	/// Set the int value with the given name
	/*
	 * Fails if the format of the member with the given name is not Coral::UniformFormat::INT32.
	 */
	bool setScalar(std::string_view name, int value, uint32_t element = 0);

	/// Set the float value with the given name
	/*
	 * Fails if the format of the member with the given name is not Coral::UniformFormat::FLOAT.
	 */
	bool setScalar(std::string_view name, float value, uint32_t element = 0);

	// Set the 2-component floating-point vector at the given index
	/*
	 * Fails if the format of the member with the given name is not Coral::UniformFormat::VEC2F.
	 */
	template<typename Vec2F>
	bool setVec2F(size_t index, const Vec2F& value, uint32_t element = 0);

	// Set the 3-component floating-point vector at the given index
	/*
	 * Fails if the format of the member at the given index is not Coral::UniformFormat::VEC3F.
	 */
	template<typename Vec3F>
	bool setVec3F(size_t index, const Vec3F& value, uint32_t element = 0);

	// Set the 4-component floating-point vector at the given index
	/*
	 * Fails if the format of the member at the given index is not Coral::UniformFormat::VEC4F.
	 */
	template<typename Vec4F>
	bool setVec4F(size_t index, const Vec4F& value, uint32_t element = 0);

	// Set the 2-component integer vector at the given index
	/*
	 * Fails if the format of the member at the given index is not Coral::UniformFormat::VEC2I.
	 */
	template<typename Vec2I>
	bool setVec2I(size_t index, const Vec2I& value, uint32_t element = 0);

	// Set the 3-component integer vector at the given index
	/*
	 * Fails if the format of the member at the given index is not Coral::UniformFormat::VEC3I.
	 */
	template<typename Vec3I>
	bool setVec3I(size_t index, const Vec3I& value, uint32_t element = 0);

	// Set the 4-component integer vector at the given index
	/*
	 * Fails if the format of the member at the given index is not Coral::UniformFormat::VEC4I.
	 */
	template<typename Vec4I>
	bool setVec4I(size_t index, const Vec4I& value, uint32_t element = 0);

	// Set the 3x3 floating-point matrix at the given index
	/*
	 * Fails if the format of the member at the given index is not Coral::UniformFormat::MAT33F.
	 */
	template<typename Mat33F>
	bool setMat33F(size_t index, const Mat33F& value, uint32_t element = 0);

	// Set the 4x4 floating-point matrix at the given index
	/*
	 * Fails if the format of the member at the given index is not Coral::UniformFormat::MAT44F.
	 */
	template<typename Mat44F>
	bool setMat44F(size_t index, const Mat44F& value, uint32_t element = 0);

	// Set the 2-component floating-point vector with the given name
	/*
	 * Fails if the format of the member with the given name is not Coral::UniformFormat::VEC2F.
	 */
	template<typename Vec2F>
	bool setVec2F(std::string_view name, const Vec2F& value, uint32_t element = 0);

	// Set the 3-component floating-point vector with the given name
	/*
	 * Fails if the format of the member with the given name is not Coral::UniformFormat::VEC3F.
	 */
	template<typename Vec3F>
	bool setVec3F(std::string_view name, const Vec3F& value, uint32_t element = 0);

	// Set the 4-component floating-point vector with the given name
	/*
	 * Fails if the format of the member with the given name is not Coral::UniformFormat::VEC4F.
	 */
	template<typename Vec4F>
	bool setVec4F(std::string_view name, const Vec4F& value, uint32_t element = 0);

	// Set the 3x3 floating-point matrix with the given name
	/*
	 * Fails if the format of the member with the given name is not Coral::UniformFormat::MAT33F.
	 */
	template<typename Mat33F>
	bool setMat33F(std::string_view name, const Mat33F& value, uint32_t element = 0);

	// Set the 4x4 floating-point matrix with the given name
	/*
	 * Fails if the format of the member with the given name is not Coral::UniformFormat::MAT44F.
	 */
	template<typename Mat44F>
	bool setMat44F(std::string_view name, const Mat44F& value, uint32_t element = 0);

	/// Get the aligned uniform block data
	const std::span<const std::byte> data() const;

	/// Get the definition of the uniform block
	const UniformBlockDefinition& getBlockDefinition() const;

private:

	// Set the float value with the given name
	bool setValue(std::string_view name, UniformFormat type, const std::byte* value, uint32_t element);

	bool setValue(size_t index, UniformFormat type, const std::byte* value, uint32_t element);

	bool setValueUnchecked(size_t index, UniformFormat format, const std::byte* value, uint32_t element);

	UniformBlockDefinition mDefinition;

	std::vector<uint32_t> mByteOffsets;

	std::vector<std::byte> mData;
};

} // Coral


template<typename Vec2F>
bool
Coral::UniformBlockBuilder::setVec2F(size_t index, const Vec2F& value, uint32_t element)
{
	return setValue(index, UniformFormat::VEC2F, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Vec3F>
bool 
Coral::UniformBlockBuilder::setVec3F(size_t index, const Vec3F& value, uint32_t element)
{
	return setValue(index, UniformFormat::VEC3F, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Vec4F>
bool 
Coral::UniformBlockBuilder::setVec4F(size_t index, const Vec4F& value, uint32_t element)
{
	return setValue(index, UniformFormat::VEC4F, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Vec2I>
bool
Coral::UniformBlockBuilder::setVec2I(size_t index, const Vec2I& value, uint32_t element)
{
	return setValue(index, UniformFormat::VEC2I, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Vec3I>
bool
Coral::UniformBlockBuilder::setVec3I(size_t index, const Vec3I& value, uint32_t element)
{
	return setValue(index, UniformFormat::VEC3I, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Vec4I>
bool
Coral::UniformBlockBuilder::setVec4I(size_t index, const Vec4I& value, uint32_t element)
{
	return setValue(index, UniformFormat::VEC4I, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Mat33F>
bool
Coral::UniformBlockBuilder::setMat33F(size_t index, const Mat33F& value, uint32_t element)
{
	return setValue(index, UniformFormat::MAT33F, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Mat44F>
bool
Coral::UniformBlockBuilder::setMat44F(size_t index, const Mat44F& value, uint32_t element)
{
	return setValue(index, UniformFormat::MAT44F, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Vec2F>
bool 
Coral::UniformBlockBuilder::setVec2F(std::string_view name, const Vec2F& value, uint32_t element)
{
	return setValue(name, UniformFormat::VEC2F, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Vec3F>
bool 
Coral::UniformBlockBuilder::setVec3F(std::string_view name, const Vec3F& value, uint32_t element)
{
	return setValue(name, UniformFormat::VEC3F, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Vec4F>
bool 
Coral::UniformBlockBuilder::setVec4F(std::string_view name, const Vec4F& value, uint32_t element)
{
	return setValue(name, UniformFormat::VEC4F, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Mat33F>
bool
Coral::UniformBlockBuilder::setMat33F(std::string_view name, const Mat33F& value, uint32_t element)
{
	return setValue(name, UniformFormat::MAT33F, reinterpret_cast<const std::byte*>(&value), element);
}


template<typename Mat44F>
bool 
Coral::UniformBlockBuilder::setMat44F(std::string_view name, const Mat44F& value, uint32_t element)
{
	return setValue(name, UniformFormat::MAT44F, reinterpret_cast<const std::byte*>(&value), element);
}

#endif // !CORAL_UNIFORMBLOCKBUILDER_HPP

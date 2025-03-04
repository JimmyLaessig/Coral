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

	UniformBlockBuilder(const UniformBlockDefinition& description);

	bool setScalar(size_t index, bool value);

	bool setScalar(size_t index, int value);

	bool setScalar(size_t index, float value);

	bool setScalar(std::string_view name, bool value);

	bool setScalar(std::string_view name, int value);

	bool setScalar(std::string_view name, float value);

	bool setValue(std::string_view name, UniformFormat type, const std::byte* value);

	bool setValue(size_t index, UniformFormat type, const std::byte* value);

	template<typename Vec2F>
	bool setVec2F(size_t index, const Vec2F& value);

	template<typename Vec3F>
	bool setVec3F(size_t index, const Vec3F& value);

	template<typename Vec4F>
	bool setVec4F(size_t index, const Vec4F& value);

	template<typename Vec2I>
	bool setVec2I(size_t index, const Vec2I& value);

	template<typename Vec3I>
	bool setVec3I(size_t index, const Vec3I& value);

	template<typename Vec4I>
	bool setVec4I(size_t index, const Vec4I& value);

	template<typename Mat33F>
	bool setMat33F(size_t index, const Mat33F& value);

	template<typename Mat44F>
	bool setMat44F(size_t index, const Mat44F& value);

	template<typename Vec2F>
	bool setVec2F(std::string_view name, const Vec2F& value);

	template<typename Vec3F>
	bool setVec3F(std::string_view name, const Vec3F& value);

	template<typename Vec4F>
	bool setVec4F(std::string_view name, const Vec4F& value);

	template<typename Mat33F>
	bool setMat33F(std::string_view name, const Mat33F& value);

	template<typename Mat44F>
	bool setMat44F(std::string_view name, const Mat44F& value);

	const std::span<const std::byte> data() const;

	const UniformBlockDefinition& getBlockDefinition() const;

	size_t size() const;

	const void* get() const;

private:

	UniformBlockDefinition mDefinition;

	std::vector<uint32_t> mByteOffsets;

	std::vector<std::byte> mData;
};

} // Coral


template<typename Vec2F>
bool
Coral::UniformBlockBuilder::setVec2F(size_t index, const Vec2F& value)
{
	return setValue(index, UniformFormat::VEC2F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec3F>
bool 
Coral::UniformBlockBuilder::setVec3F(size_t index, const Vec3F& value)
{
	return setValue(index, UniformFormat::VEC3F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec4F>
bool 
Coral::UniformBlockBuilder::setVec4F(size_t index, const Vec4F& value)
{
	return setValue(index, UniformFormat::VEC4F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec2I>
bool
Coral::UniformBlockBuilder::setVec2I(size_t index, const Vec2I& value)
{
	return setValue(index, UniformFormat::VEC2I, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec3I>
bool
Coral::UniformBlockBuilder::setVec3I(size_t index, const Vec3I& value)
{
	return setValue(index, UniformFormat::VEC3I, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec4I>
bool
Coral::UniformBlockBuilder::setVec4I(size_t index, const Vec4I& value)
{
	return setValue(index, UniformFormat::VEC4I, reinterpret_cast<const std::byte*>(&value));
}


template<typename Mat33F>
bool
Coral::UniformBlockBuilder::setMat33F(size_t index, const Mat33F& value)
{
	return setValue(index, UniformFormat::MAT33F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Mat44F>
bool
Coral::UniformBlockBuilder::setMat44F(size_t index, const Mat44F& value)
{
	return setValue(index, UniformFormat::MAT44F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec2F>
bool 
Coral::UniformBlockBuilder::setVec2F(std::string_view name, const Vec2F& value)
{
	return setValue(name, UniformFormat::VEC2F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec3F>
bool 
Coral::UniformBlockBuilder::setVec3F(std::string_view name, const Vec3F& value)
{
	return setValue(name, UniformFormat::VEC3F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec4F>
bool 
Coral::UniformBlockBuilder::setVec4F(std::string_view name, const Vec4F& value)
{
	return setValue(name, UniformFormat::VEC4F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Mat33F>
bool
Coral::UniformBlockBuilder::setMat33F(std::string_view name, const Mat33F& value)
{
	return setValue(name, UniformFormat::MAT33F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Mat44F>
bool 
Coral::UniformBlockBuilder::setMat44F(std::string_view name, const Mat44F& value)
{
	return setValue(name, UniformFormat::MAT44F, reinterpret_cast<const std::byte*>(&value));
}

#endif // !CORAL_UNIFORMBLOCKBUILDER_HPP
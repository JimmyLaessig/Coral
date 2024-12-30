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

	bool setScalar(std::string_view name, ValueType type, const std::byte* value);

	bool setScalar(size_t index, ValueType type, const std::byte* value);

	template<typename Vec2F>
	bool setVec2F(size_t index, const Vec2F& value);

	template<typename Vec3F>
	bool setVec3F(size_t index, const Vec3F& value);

	template<typename Vec4F>
	bool setVec4F(size_t index, const Vec4F& value);

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

	std::vector<size_t> mByteOffsets;

	std::vector<std::byte> mData;
};

} // Coral


template<typename Vec2F>
bool
Coral::UniformBlockBuilder::setVec2F(size_t index, const Vec2F& value)
{
	return setScalar(index, ValueType::VEC2F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec3F>
bool 
Coral::UniformBlockBuilder::setVec3F(size_t index, const Vec3F& value)
{
	return setScalar(index, ValueType::VEC3F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec4F>
bool 
Coral::UniformBlockBuilder::setVec4F(size_t index, const Vec4F& value)
{
	return setScalar(index, ValueType::VEC4F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Mat33F>
bool
Coral::UniformBlockBuilder::setMat33F(size_t index, const Mat33F& value)
{
	return setScalar(index, ValueType::MAT33F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Mat44F>
bool
Coral::UniformBlockBuilder::setMat44F(size_t index, const Mat44F& value)
{
	return setScalar(index, ValueType::MAT44F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec2F>
bool 
Coral::UniformBlockBuilder::setVec2F(std::string_view name, const Vec2F& value)
{
	return setScalar(name, ValueType::VEC2F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec3F>
bool 
Coral::UniformBlockBuilder::setVec3F(std::string_view name, const Vec3F& value)
{
	return setScalar(name, ValueType::VEC3F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Vec4F>
bool 
Coral::UniformBlockBuilder::setVec4F(std::string_view name, const Vec4F& value)
{
	return setScalar(name, ValueType::VEC4F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Mat33F>
bool 
Coral::UniformBlockBuilder::setMat33F(std::string_view name, const Mat33F& value)
{
	return setScalar(name, ValueType::MAT33F, reinterpret_cast<const std::byte*>(&value));
}


template<typename Mat44F>
bool 
Coral::UniformBlockBuilder::setMat44F(std::string_view name, const Mat44F& value)
{
	return setScalar(name, ValueType::MAT44F, reinterpret_cast<const std::byte*>(&value));
}

#endif // !CORAL_UNIFORMBLOCKBUILDER_HPP
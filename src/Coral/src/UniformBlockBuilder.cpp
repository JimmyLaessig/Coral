#include <Coral/UniformBlockBuilder.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

using namespace Coral;


constexpr size_t
byteStride(ValueType type)
{
	switch (type)
	{
		case ValueType::BOOL:	return 4;
		case ValueType::INT:	return 4;
		case ValueType::FLOAT:	return 4;
		case ValueType::VEC2F:	return 8;
		case ValueType::VEC3F:	return 16;
		case ValueType::VEC4F:	return 16;
		case ValueType::MAT33F:	return 48;
		case ValueType::MAT44F:	return 64;
	};

	std::unreachable();
}


constexpr size_t
byteSize(ValueType type)
{
	switch (type)
	{
		case ValueType::BOOL:	return 4;
		case ValueType::INT:	return 4;
		case ValueType::FLOAT:	return 4;
		case ValueType::VEC2F:	return 8;
		case ValueType::VEC3F:	return 12;
		case ValueType::VEC4F:	return 16;
		case ValueType::MAT33F:	return 36;
		case ValueType::MAT44F:	return 64;
	};

	std::unreachable();
}


UniformBlockBuilder::UniformBlockBuilder(const UniformBlockDefinition& definition)
	: mDefinition(definition)
{
	size_t bufferSize{ 0 };
	for (size_t i = 0; i < mDefinition.members.size(); ++i)
	{
		mByteOffsets.push_back(bufferSize);
		bufferSize += byteStride(mDefinition.members[i].type);
	}

	mData.resize(bufferSize, std::byte(0));
}


bool
UniformBlockBuilder::setScalar(size_t index, bool value)
{
	int v = value;
	return setScalar(index, ValueType::BOOL, reinterpret_cast<const std::byte*>(&v));
}


bool
UniformBlockBuilder::setScalar(size_t index, int value)
{
	return setScalar(index, ValueType::INT, reinterpret_cast<const std::byte*>(&value));
}


bool
UniformBlockBuilder::setScalar(size_t index, float value)
{
	return setScalar(index, ValueType::FLOAT, reinterpret_cast<const std::byte*>(&value));
}


bool
UniformBlockBuilder::setScalar(std::string_view name, bool value)
{
	int v = value;
	return setScalar(name, ValueType::BOOL, reinterpret_cast<const std::byte*>(&v));
}


bool
UniformBlockBuilder::setScalar(std::string_view name, int value)
{
	return setScalar(name, ValueType::INT, reinterpret_cast<const std::byte*>(&value));
}


bool
UniformBlockBuilder::setScalar(std::string_view name, float value)
{
	return setScalar(name, ValueType::FLOAT, reinterpret_cast<const std::byte*>(&value));
}


bool
UniformBlockBuilder::setScalar(size_t index, ValueType type, const std::byte* value)
{
	if (index >= mDefinition.members.size())
	{
		return false;
	}

	if (type != mDefinition.members[index].type)
	{
		return false;
	}

	auto dest = mData.data() + mByteOffsets[index];

	if (type == ValueType::MAT33F)
	{
		constexpr size_t size = byteSize(ValueType::VEC3F);
		constexpr size_t stride = byteSize(ValueType::VEC4F);

		for (size_t i = 0; i < 3; ++i)
		{
			std::memcpy(dest + stride * i, value + size * i, size);
		}
	}
	else
	{
		std::memcpy(dest, value, byteSize(type));
	}

	return true;
}


bool
UniformBlockBuilder::setScalar(std::string_view name, ValueType type, const std::byte* value)
{
	auto iter = std::find_if(mDefinition.members.begin(), mDefinition.members.end(),
						     [&](const auto& member) { return member.name == name; });

	if (iter == mDefinition.members.end())
	{
		return false;
	}

	auto index = std::distance(mDefinition.members.begin(), iter);

	return setScalar(index, type, value);
}


const std::span<const std::byte>
UniformBlockBuilder::data() const
{
	return std::span<const std::byte>(mData.data(), mData.size());
}


const UniformBlockDefinition&
UniformBlockBuilder::getBlockDefinition() const
{
	return mDefinition;
}


size_t
UniformBlockBuilder::size() const
{
	return mData.size();
}


const void*
UniformBlockBuilder::get() const
{
	return mData.data();
}


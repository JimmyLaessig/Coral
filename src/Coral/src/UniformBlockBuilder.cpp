#include <Coral/UniformBlockBuilder.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <ranges>

using namespace Coral;


constexpr uint8_t
alignment(UniformFormat type)
{
	switch (type)
	{
		case UniformFormat::BOOL:	return 4;
		case UniformFormat::INT32:	return 4;
		case UniformFormat::FLOAT:	return 4;
		case UniformFormat::VEC2I:	return 8;
		case UniformFormat::VEC3I:	return 16;
		case UniformFormat::VEC4I:	return 16;
		case UniformFormat::VEC2F:	return 8;
		case UniformFormat::VEC3F:	return 16;
		case UniformFormat::VEC4F:	return 16;
		case UniformFormat::MAT33F:	return 16;
		case UniformFormat::MAT44F:	return 16;
	};

	std::unreachable();
}

constexpr uint32_t
nextMultipleOf(uint32_t n, uint32_t v)
{
	return ((v + n - 1) / n) * n;
}


UniformBlockBuilder::UniformBlockBuilder(const UniformBlockDefinition& definition)
	: mDefinition(definition)
{
	uint32_t bufferSize{ 0 };

	for (const auto[i, member] : std::views::enumerate(mDefinition.members))
	{
		// Advance the beginning of the next member so that the required alignment of the member is met
		bufferSize = nextMultipleOf(alignment(member.type), bufferSize);
		// Store the current offset to the beginning of the member
		mByteOffsets.push_back(bufferSize);
		// Advance the buffer size
		bufferSize += sizeInBytes(member.type);
	}

	mData.resize(bufferSize, std::byte(0));
}


bool
UniformBlockBuilder::setScalar(size_t index, bool value)
{
	int v = value;
	return setValue(index, UniformFormat::BOOL, reinterpret_cast<const std::byte*>(&v));
}


bool
UniformBlockBuilder::setScalar(size_t index, int value)
{
	return setValue(index, UniformFormat::INT32, reinterpret_cast<const std::byte*>(&value));
}


bool
UniformBlockBuilder::setScalar(size_t index, float value)
{
	return setValue(index, UniformFormat::FLOAT, reinterpret_cast<const std::byte*>(&value));
}


bool
UniformBlockBuilder::setScalar(std::string_view name, bool value)
{
	int v = value;
	return setValue(name, UniformFormat::BOOL, reinterpret_cast<const std::byte*>(&v));
}


bool
UniformBlockBuilder::setScalar(std::string_view name, int value)
{
	return setValue(name, UniformFormat::INT32, reinterpret_cast<const std::byte*>(&value));
}


bool
UniformBlockBuilder::setScalar(std::string_view name, float value)
{
	return setValue(name, UniformFormat::FLOAT, reinterpret_cast<const std::byte*>(&value));
}


bool
UniformBlockBuilder::setValue(size_t index, UniformFormat type, const std::byte* value)
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

	if (type == UniformFormat::MAT33F)
	{
		constexpr size_t size = sizeInBytes(UniformFormat::VEC3F);
		constexpr size_t stride = sizeInBytes(UniformFormat::VEC4F);

		for (size_t i = 0; i < 3; ++i)
		{
			std::memcpy(dest + stride * i, value + size * i, size);
		}
	}
	else
	{
		std::memcpy(dest, value, sizeInBytes(type));
	}

	return true;
}


bool
UniformBlockBuilder::setValue(std::string_view name, UniformFormat type, const std::byte* value)
{
	auto iter = std::ranges::find_if(mDefinition.members,
								     [&](const auto& member) { return member.name == name; });

	if (iter == mDefinition.members.end())
	{
		return false;
	}

	auto index = std::distance(mDefinition.members.begin(), iter);

	return setValue(index, type, value);
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


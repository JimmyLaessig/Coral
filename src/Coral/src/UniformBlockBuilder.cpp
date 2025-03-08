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


namespace std140
{

struct Layout
{
	// The alignment of the 
	uint32_t alignment;
	// The size of the member in bytes
	uint32_t size;
	// Distance in bytes between two consecutive array elements in the member
	uint32_t stride;
};


Layout
getMemberLayout(const Coral::MemberDefinition& member)
{
	// std140 layout specs: https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf#page=159
	// 
	// 1. If the member is a scalar consuming N basic machine units, the base alignment is N.
	// 2. If the member is a two- or four-component vector with components consuming N basic machine units, the base 
	//    alignment is 2N or 4N, respectively
	// 3. If the member is a three-component vector with components consuming N basic machine units, the base alignment
	//    is 4N.
	// 4. If the member is an array of scalars or vectors, the base alignment and array stride are set to match the
	//    base alignment of a single array element, according to rules (1), (2), and (3), and rounded up to the base
	//    alignment of a vec4. The array may have padding at the end; the base offset of the member following the array
	//    is rounded up to the next multiple of the base alignment.
	// 5. If the member is a column-major matrix with C columns and R rows, the matrix is stored identically to an
	//    array of C column vectors with R components each, according to rule (4).
	// 6. If the member is an array of S column - major matrices with C columns and R rows, the matrix is stored
	//    identically to a row of S × C column vectors with R components each, according to rule(4).

	// We consume 4 machine units (float32, int32)
	constexpr uint32_t N = 4;
	auto c = member.count;
	switch (member.type)
	{
		case UniformFormat::BOOL:	return { N,     c * N,      N };
		case UniformFormat::INT32:	return { N,     c * N,      N };
		case UniformFormat::FLOAT:	return { N,     c * N,      N };
		case UniformFormat::VEC2I:	return { N * 2, c * N * 2,  N * 2 };
		case UniformFormat::VEC2F:	return { N * 2, c * N * 2,  N * 2 };
		case UniformFormat::VEC3I:	return { N * 4, c * N * 3,  N * 4 };
		case UniformFormat::VEC3F:	return { N * 4, c * N * 3,  N * 4 };
		case UniformFormat::VEC4I:	return { N * 4, c * N * 4,  N * 4 };
		case UniformFormat::VEC4F:	return { N * 4, c * N * 4,  N * 4 };
		case UniformFormat::MAT33F:	return { N * 4, c * N * 12, N * 12 };
		case UniformFormat::MAT44F:	return { N * 4, c * N * 16, N * 16 };
	}

	std::unreachable();
}


} // namespace std140


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
		auto [alignment, size, stride] = std140::getMemberLayout(member);

		// Advance the beginning of the next member so that the required alignment of the member is met
		bufferSize = nextMultipleOf(alignment, bufferSize);
		// Store the current offset to the beginning of the member
		mByteOffsets.push_back(bufferSize);
		// Advance the buffer size
		bufferSize += size;
	}

	mData.resize(bufferSize, std::byte(0));
}


bool
UniformBlockBuilder::setScalar(size_t index, bool value, uint32_t element)
{
	int v = value;
	return setValue(index, UniformFormat::BOOL, reinterpret_cast<const std::byte*>(&v), element);
}


bool
UniformBlockBuilder::setScalar(size_t index, int value, uint32_t element)
{
	return setValue(index, UniformFormat::INT32, reinterpret_cast<const std::byte*>(&value), element);
}


bool
UniformBlockBuilder::setScalar(size_t index, float value, uint32_t element)
{
	return setValue(index, UniformFormat::FLOAT, reinterpret_cast<const std::byte*>(&value), element);
}


bool
UniformBlockBuilder::setScalar(std::string_view name, bool value, uint32_t element)
{
	int v = value;
	return setValue(name, UniformFormat::BOOL, reinterpret_cast<const std::byte*>(&v), element);
}


bool
UniformBlockBuilder::setScalar(std::string_view name, int value, uint32_t element)
{
	return setValue(name, UniformFormat::INT32, reinterpret_cast<const std::byte*>(&value), element);
}


bool
UniformBlockBuilder::setScalar(std::string_view name, float value, uint32_t element)
{
	return setValue(name, UniformFormat::FLOAT, reinterpret_cast<const std::byte*>(&value), element);
}


bool
UniformBlockBuilder::setValue(size_t index, UniformFormat format, const std::byte* value, uint32_t element)
{
	if (index >= mDefinition.members.size())
	{
		return false;
	}

	if (format != mDefinition.members[index].type || 
		element >= mDefinition.members[index].count)
	{
		return false;
	}

	return setValueUnchecked(index, format, value, element);
}


bool
UniformBlockBuilder::setValue(std::string_view name, UniformFormat type, const std::byte* value, uint32_t element)
{
	auto iter = std::ranges::find_if(mDefinition.members,
									 [&](const auto& member) { return member.name == name && element < member.count; });

	if (iter == mDefinition.members.end())
	{
		return false;
	}

	auto index = std::distance(mDefinition.members.begin(), iter);

	return setValueUnchecked(index, type, value, element);
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


bool
UniformBlockBuilder::setValueUnchecked(size_t index, UniformFormat format, const std::byte* value, uint32_t element)
{
	auto [_, size, stride] = std140::getMemberLayout(mDefinition.members[index]);

	auto dstPtr = mData.data();
	auto dstOffset = mByteOffsets[index] + element * stride;
	assert(dstOffset + size <= mData.size());

	if (format == UniformFormat::MAT33F)
	{
		auto* m = reinterpret_cast<const float*>(value);

		std::array<float, 12> padded =
		{
			m[0], m[1], m[2], 0.f,
			m[3], m[4], m[5], 0.f,
			m[6], m[7], m[8], 0.f,
		};

		std::memcpy(dstPtr + dstOffset, padded.data(), size);

		return true;
	}

	std::memcpy(dstPtr + dstOffset, value, size);

	return true;
}

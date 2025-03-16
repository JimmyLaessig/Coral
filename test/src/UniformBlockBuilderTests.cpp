#include "Catch.hpp"

#include <glm/glm.hpp>

#include <Coral/UniformBlockBuilder.hpp>

float r()
{
	static float counter = 1.f;
	return counter++;
}

TEST_CASE("Test UniformBlockBuilder")
{
	struct UniformBlock
	{
		// Float member
		float m1{ r() };
		// 4X4 matrix member (must be aligned to a multiple of 16)
		glm::vec3 a1{ 0.f, 0.f, 0.f };
		glm::mat4 m2{ r(), r(), r(), r(),
					  r(), r(), r(), r(),
					  r(), r(), r(), r(),
					  r(), r(), r(), r() };
		// Vec2 member (must be aligned to a multiple of 8)
		glm::vec2 m3{ r(), r() };
		//// 3x3 matrix member (must be aligned to a multiple of 16 and 4 byte padding for each colum vector)
		glm::vec2 a2{ 0.f, 0.f };
		glm::mat3x4 m4{ r(),  r(),  r(), 0.f,
						r(),  r(),  r(), 0.f,
						r(),  r(),  r(), 0.f };
		// Vec3 member (must be aligned to a multiple of 16)
		glm::vec3 m5{ r(), r(), r() };
		// Float member (must be aligned to a multiple of 4)
		float m6{ r() };

		// Vec4 member (must be aligned to multiple of 16)
		glm::vec4 m7{ r(), r(), r(), r() };

		// Vec2 member (must be aligned to multiple of 8)
		glm::vec2 m8{ r(), r() };

		//// Vec3 member (must be aligned to multiple of 16)
		glm::vec2 a5{ 0.f, 0.f };
		glm::vec3 m9{ r(), r(), r() };
	};

	UniformBlock expected{};
	Coral::UniformBlockDefinition definition =
	{
		{
			{ Coral::UniformFormat::FLOAT,  "m1", 1 },
			{ Coral::UniformFormat::MAT44F, "m2", 1 },
			{ Coral::UniformFormat::VEC2F,  "m3", 1 },
			{ Coral::UniformFormat::MAT33F, "m4", 1 },
			{ Coral::UniformFormat::VEC3F,  "m5", 1 },
			{ Coral::UniformFormat::FLOAT,  "m6", 1 },
			{ Coral::UniformFormat::VEC4F,  "m7", 1 },
			{ Coral::UniformFormat::VEC2F,  "m8", 1 },
			{ Coral::UniformFormat::VEC3F,  "m9", 1 },
		}
	};

	Coral::UniformBlockBuilder builder(definition);

	CHECK(builder.setScalar("m1", expected.m1));
	CHECK(builder.setMat44F("m2", expected.m2));
	CHECK(builder.setVec2F("m3", expected.m3));
	CHECK(builder.setMat33F("m4", glm::mat3(expected.m4)));
	CHECK(builder.setVec3F("m5", expected.m5));
	CHECK(builder.setScalar("m6", expected.m6));
	CHECK(builder.setVec4F("m7", expected.m7));
	CHECK(builder.setVec2F("m8", expected.m8));
	CHECK(builder.setVec3F("m9", expected.m9));

	auto data = builder.data();

	UniformBlock result;
	REQUIRE(data.size() == sizeof(result));
	std::memset(&result, 0, sizeof(result));
	std::memcpy(&result, data.data(), sizeof(result));

	CHECK(expected.m1 == result.m1);
	CHECK(expected.m2 == result.m2);
	CHECK(expected.m3 == result.m3);
	CHECK(glm::mat3(expected.m4) == glm::mat3(result.m4));
	CHECK(expected.m5 == result.m5);
	CHECK(expected.m6 == result.m6);
	CHECK(expected.m7 == result.m7);
	CHECK(expected.m8 == result.m8);
	CHECK(expected.m9 == result.m9);
}

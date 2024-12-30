#ifndef CORAL_RESOURCES_CUBE_HPP
#define CORAL_RESOURCES_CUBE_HPP

#include <glm/glm.hpp>

#include <array>

namespace Cube
{

constexpr std::array<glm::vec3, 24> Positions =
{
	// -z
	glm::vec3{  .5f, -.5f, -.5f },
	glm::vec3{ -.5f, -.5f, -.5f },
	glm::vec3{  .5f,  .5f, -.5f },
	glm::vec3{ -.5f,  .5f, -.5f },

	// +z
	glm::vec3{ -.5f, -.5f,  .5f },
	glm::vec3{  .5f, -.5f,  .5f },
	glm::vec3{ -.5f,  .5f,  .5f },
	glm::vec3{  .5f,  .5f,  .5f },

	// -x
	glm::vec3{ -.5f, -.5f, -.5f },
	glm::vec3{ -.5f, -.5f,  .5f },
	glm::vec3{ -.5f,  .5f, -.5f },
	glm::vec3{ -.5f,  .5f,  .5f },

	// +x
	glm::vec3{ .5f, -.5f,  .5f },
	glm::vec3{ .5f, -.5f, -.5f },
	glm::vec3{ .5f,  .5f,  .5f },
	glm::vec3{ .5f,  .5f, -.5f },

	// -y
	glm::vec3{ -.5f, -.5f, -.5f },
	glm::vec3{  .5f, -.5f, -.5f },
	glm::vec3{ -.5f, -.5f,  .5f },
	glm::vec3{  .5f, -.5f,  .5f },

	// +y
	glm::vec3{ -.5f, .5f,  .5f },
	glm::vec3{  .5f, .5f,  .5f },
	glm::vec3{ -.5f, .5f, -.5f },
	glm::vec3{  .5f, .5f, -.5f },
};


constexpr std::array<glm::vec2, 24> Texcoords =
{
	// -z
	glm::vec2{ 0.f, 0.f },
	glm::vec2{ 1.f, 0.f },
	glm::vec2{ 0.f, 1.f },
	glm::vec2{ 1.f, 1.f },

	// +z
	glm::vec2{ 0.f, 0.f },
	glm::vec2{ 1.f, 0.f },
	glm::vec2{ 0.f, 1.f },
	glm::vec2{ 1.f, 1.f },

	// -x
	glm::vec2{ 0.f, 0.f },
	glm::vec2{ 1.f, 0.f },
	glm::vec2{ 0.f, 1.f },
	glm::vec2{ 1.f, 1.f },

	// +x
	glm::vec2{ 0.f, 0.f },
	glm::vec2{ 1.f, 0.f },
	glm::vec2{ 0.f, 1.f },
	glm::vec2{ 1.f, 1.f },

	// -y
	glm::vec2{ 0.f, 0.f },
	glm::vec2{ 1.f, 0.f },
	glm::vec2{ 0.f, 1.f },
	glm::vec2{ 1.f, 1.f },

	// +y
	glm::vec2{ 0.f, 0.f },
	glm::vec2{ 1.f, 0.f },
	glm::vec2{ 0.f, 1.f },
	glm::vec2{ 1.f, 1.f },
};


constexpr std::array<glm::vec3, 24> Normals =
{
	// -z
	glm::vec3{ 0.f, 0.f, -1.f },
	glm::vec3{ 0.f, 0.f, -1.f },
	glm::vec3{ 0.f, 0.f, -1.f },
	glm::vec3{ 0.f, 0.f, -1.f },

	// +z
	glm::vec3{ 0.f, 0.f, 1.f },
	glm::vec3{ 0.f, 0.f, 1.f },
	glm::vec3{ 0.f, 0.f, 1.f },
	glm::vec3{ 0.f, 0.f, 1.f },

	// -x
	glm::vec3{ -1.f, 0.f, 0.f },
	glm::vec3{ -1.f, 0.f, 0.f },
	glm::vec3{ -1.f, 0.f, 0.f },
	glm::vec3{ -1.f, 0.f, 0.f },

	// +x
	glm::vec3{ 1.f, 0.f, 0.f },
	glm::vec3{ 1.f, 0.f, 0.f },
	glm::vec3{ 1.f, 0.f, 0.f },
	glm::vec3{ 1.f, 0.f, 0.f },

	// -y
	glm::vec3{ 0.f, -1.f, 0.f },
	glm::vec3{ 0.f, -1.f, 0.f },
	glm::vec3{ 0.f, -1.f, 0.f },
	glm::vec3{ 0.f, -1.f, 0.f },

	// +y
	glm::vec3{ 0.f, 1.f, 0.f },
	glm::vec3{ 0.f, 1.f, 0.f },
	glm::vec3{ 0.f, 1.f, 0.f },
	glm::vec3{ 0.f, 1.f, 0.f },
};


constexpr std::array<uint16_t, 36> Indices =
{
	// -z
	0, 1, 2,
	2, 1, 3,

	// +z
	4, 5, 6,
	6, 5, 7,

	// -x
	8, 9, 10,
	10, 9, 11,

	// +x
	12, 13, 14,
	14, 13, 15,

	// -y
	16, 17, 18,
	18, 17, 19,

	// +y
	20, 21, 22,
	22, 21, 23,
};

} // namespace Cube

#endif // !CORAL_RESOURCES_CUBE_HPP

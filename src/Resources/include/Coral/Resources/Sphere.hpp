#ifndef CORAL_RESOURCES_SPHERE_HPP
#define CORAL_RESOURCES_SPHERE_HPP

#include <glm/glm.hpp>

#include <array>
#include <vector>

#pragma once

struct Sphere
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<uint16_t> indices;

    static inline Sphere Generate(uint8_t stacks = 9, uint8_t slices = 10, float radius = 0.5f)
    {
        Sphere s;
        const float PI = 3.14159265358979323846f;

        // vertices: (stacks+1) * (slices+1)
        for (uint8_t i = 0; i <= stacks; ++i)
        {
            float v = static_cast<float>(i) / static_cast<float>(stacks);
            float theta = v * PI; // 0..PI
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            for (uint8_t j = 0; j <= slices; ++j)
            {
                float u = static_cast<float>(j) / static_cast<float>(slices);
                float phi = u * 2.0f * PI; // 0..2PI
                float sinPhi = std::sin(phi);
                float cosPhi = std::cos(phi);

                glm::vec3 normal{ sinTheta * cosPhi, cosTheta, sinTheta * sinPhi };
                glm::vec3 position = normal * radius;
                s.positions.push_back(position);
                s.normals.push_back(normal);
                s.texcoords.push_back(glm::vec2{ u, v });
            }
        }

        // indices
        for (uint8_t i = 0; i < stacks; ++i)
        {
            for (uint8_t j = 0; j < slices; ++j)
            {
                uint16_t first  = static_cast<uint16_t>(i * (slices + 1) + j);
                uint16_t second = static_cast<uint16_t>((i + 1) * (slices + 1) + j);

                // triangle 1
                s.indices.push_back(first);
                s.indices.push_back(first + 1);
                s.indices.push_back(second);

                // triangle 2
                s.indices.push_back(second);
                s.indices.push_back(first + 1);
                s.indices.push_back(second + 1);
            }
        }

        return s;
    }
}; // struct Sphere

#endif // !CORAL_RESOURCES_SPHERE_HPP

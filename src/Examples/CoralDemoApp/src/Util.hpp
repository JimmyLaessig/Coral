#ifndef UTIL_HPP
#define UTIL_HPP

#include <glm/glm.hpp>

#include <Coral/Coral.h>

#include <Coral/Util/RAII.hpp>
#include <Coral/Util/UniformBlockBuilder.hpp>

#include <chrono>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace Util
{

struct Mesh
{
    Coral::BufferPtr mIndexBuffer;
    Coral::BufferPtr mPositionBuffer;
    Coral::BufferPtr mNormalBuffer;
    Coral::BufferPtr mTexcoordBuffer;
    CoIndexFormat mIndexFormat{ CO_INDEX_FORMAT_UINT16 };
    uint32_t mIndexCount{ 0 };

}; // class Mesh

struct Texture
{
    Coral::ImagePtr mImage;
    Coral::SamplerPtr mSampler;
};

struct Material
{
    Texture mBaseColorTexture;
};

using UniformBlockBuilder = Coral::UniformBlockBuilder<glm::vec2, glm::vec3, glm::vec4, glm::ivec2, glm::ivec3, glm::ivec4, glm::mat3, glm::mat4>;

/*!
 * Compile slang shader to SPIRV
 */
std::optional<std::vector<CoByte>> compileShader(const std::string& slangSource, const std::string& entryPoint);

/*!
 *
 */
struct Image
{
    std::vector<std::byte> data;
    uint32_t width{ 0 };
    uint32_t height{ 0 };
    CoPixelFormat format{ CO_PIXEL_FORMAT_RGBA8_UI };
};

/*!
 * Load the image from a buffer
 */
std::optional<Image> loadImage(std::span<const uint8_t> buffer);

/*!
 * Create a buffer of the given type filled with the given data
 */
Coral::BufferPtr createBuffer(CoContext context, std::span<const std::byte> data, CoBufferType type, bool cpuVisible = false);

/*!
 * \brief Update the buffer with the given data
 */
void updateBuffer(CoContext context, CoBuffer buffer, std::span<const std::byte> data);

/*!
 * \brief Create a uniform buffer
 */
Coral::BufferPtr createUniformBuffer(CoContext context, const UniformBlockBuilder& block);

/*!
 * \brief Update the uniform buffer using the block
 */
//void updateUniformBuffer(CoBuffer buffer, const UniformBlockBuilder& block);

/*!
 * \brief Create a texture from the image data
 */
Texture createTexture(CoContext context, std::span<const uint8_t> buffer);

} // namespace Util

#endif // !UTIL_HPP

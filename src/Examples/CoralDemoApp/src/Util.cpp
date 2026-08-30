#include "Util.hpp"

#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <slang.h>

#include <iostream>

namespace
{

template<typename T>
struct SlangDeleter
{
    void operator()(T* obj)
    {
        obj->release();
    }
};

template<typename T>
using SlangPtr = std::unique_ptr<T, SlangDeleter<T>>;

}

namespace Util
{

std::optional<std::vector<CoByte>>
compileShader(const std::string& slangSource, const std::string& entryPoint)
{
    SlangPtr<slang::IGlobalSession> globalSession;
    slang::createGlobalSession(std::out_ptr(globalSession));

    if (!globalSession)
    {
        return {};
    }

    slang::TargetDesc targetDesc
    {
        .format  = SLANG_SPIRV,
        .profile = globalSession->findProfile("spirv_1_5"),
        .flags   = 0,
    };

    slang::SessionDesc sessionDesc
    {
        .targets                  = &targetDesc,
        .targetCount              = 1,
        .defaultMatrixLayoutMode  = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
        .compilerOptionEntryCount = 0,
    };

    SlangPtr<slang::ISession> session;
    globalSession->createSession(sessionDesc, std::out_ptr(session));

    slang::IModule* slangModule = nullptr;
    {
        SlangPtr<slang::IBlob> diagnosticsBlob;

        slangModule = session->loadModuleFromSourceString("Shader",
                                                          nullptr,
                                                          slangSource.c_str(),
                                                          std::out_ptr(diagnosticsBlob));

        if (!slangModule)
        {
            if (diagnosticsBlob)
            {
                std::cerr << diagnosticsBlob->getBufferPointer() << std::endl;
            }
            return {};
        }
    }

    SlangPtr<slang::IEntryPoint> entryPoint2;
    {
        auto result = slangModule->findEntryPointByName(entryPoint.c_str(),
                                                        std::out_ptr(entryPoint2));
        if (SLANG_FAILED(result))
        {
            return {};
        }
    }

    std::vector<slang::IComponentType*> componentTypes;
    componentTypes.push_back(slangModule);
    componentTypes.push_back(entryPoint2.get());

    SlangPtr<slang::IComponentType> composedProgram;
    {
        SlangPtr<slang::IBlob> diagnosticsBlob;
        auto result = session->createCompositeComponentType(componentTypes.data(),
                                                            componentTypes.size(),
                                                            std::out_ptr(composedProgram),
                                                            std::out_ptr(diagnosticsBlob));
        if (SLANG_FAILED(result))
        {
            if (diagnosticsBlob)
            {
                std::cerr << diagnosticsBlob->getBufferPointer() << std::endl;
            }
            return {};
        }
    }

    SlangPtr<slang::IBlob> spirvCode;
    {
        SlangPtr<slang::IBlob> diagnosticsBlob;
        auto result = composedProgram->getEntryPointCode(0,
                                                         0,
                                                         std::out_ptr(spirvCode),
                                                         std::out_ptr(diagnosticsBlob));

        if (SLANG_FAILED(result))
        {
            if (diagnosticsBlob)
            {
                std::cerr << diagnosticsBlob->getBufferPointer() << std::endl;
            }
            return {};
        }
    }

    auto ptr = reinterpret_cast<const CoByte*>(spirvCode->getBufferPointer());
    return std::vector<CoByte>(ptr, ptr + spirvCode->getBufferSize());
}


std::optional<Image>
loadImage(std::span<const uint8_t> buffer)
{
    int x, y, c;
    stbi__vertically_flip_on_load = true;
    auto data = stbi_load_from_memory(buffer.data(), static_cast<int>(buffer.size()), &x, &y, &c, 0);

    if (data == nullptr)
    {
        return std::nullopt;
    }

    Image image;
    image.width  = static_cast<uint32_t>(x);
    image.height = static_cast<uint32_t>(y);
    image.data.resize(x * y * c);
    std::memcpy(image.data.data(), data, image.data.size());

    stbi_image_free(data);

    switch (1)
    {
    case 1:
        switch (c)
        {
            case 1:    image.format = CO_PIXEL_FORMAT_R8_UI;    break;
            case 2:    image.format = CO_PIXEL_FORMAT_RG8_UI;   break;
            case 3:    image.format = CO_PIXEL_FORMAT_RGB8_UI;  break;
            case 4:    image.format = CO_PIXEL_FORMAT_RGBA8_UI; break;
            default:
                return std::nullopt;
        }
        break;
    case 2:
        switch (c)
        {
            case 1:    image.format = CO_PIXEL_FORMAT_R16_UI;    break;
            case 2:    image.format = CO_PIXEL_FORMAT_RG16_UI;   break;
            case 3:    image.format = CO_PIXEL_FORMAT_RGB16_UI;  break;
            case 4:    image.format = CO_PIXEL_FORMAT_RGBA16_UI; break;
            default:
                return std::nullopt;
        }
        break;
    }

    return { std::move(image) };
}


Coral::BufferPtr
createBuffer(CoContext context, std::span<const std::byte> data, CoBufferType type, bool cpuVisible)
{
    CoBufferCreateConfig bufferConfig
    {
        .size       = static_cast<uint32_t>(data.size()),
        .type       = type,
        .cpuVisible = cpuVisible,
    };

    Coral::BufferPtr buffer;
    if (coContextCreateBuffer(context, &bufferConfig, std::out_ptr(buffer)) != CO_SUCCESS)
    {
        return nullptr;
    }

    updateBuffer(context, buffer.get(), data);

    return buffer;
}


void
updateBuffer(CoContext context, CoBuffer buffer, std::span<const std::byte> data)
{
    CoByte* mapped{ nullptr };
    if (coBufferMap(buffer, &mapped) == CO_SUCCESS)
    {
        std::memcpy(mapped, data.data(), data.size());
        coBufferUnMap(buffer);
        return;
    }

    CoCommandQueue queue{ nullptr };
    if (coContextGetTransferQueue(context, &queue) != CO_SUCCESS)
    {
        return;
    }

    CoCommandBufferCreateConfig commandBufferConfig{};
    Coral::CommandBufferPtr commandBuffer;
    if (coCommandQueueCreateCommandBuffer(queue, &commandBufferConfig, std::out_ptr(commandBuffer)) != CO_SUCCESS)
    {
        return;
    }

    CoUpdateBufferDataInfo updateInfo
    {
        .buffer = buffer,
        .offset = 0,
        .pData = reinterpret_cast<const CoByte*>(data.data()),
        .dataCount = static_cast<uint32_t>(data.size_bytes()),
    };

    coCommandBufferBegin(commandBuffer.get());
    coCommandBufferUpdateBufferData(commandBuffer.get(), &updateInfo);
    coCommandBufferEnd(commandBuffer.get());

    auto cbPtr = commandBuffer.get();

    CoCommandBufferSubmitInfo submitInfo
    {
        .pCommandBuffers = &cbPtr,
        .commandBufferCount = 1,
    };

    Coral::FencePtr fence;
    CoFenceCreateConfig fenceConfig{};
    if (coContextCreateFence(context, &fenceConfig, std::out_ptr(fence)) != CO_SUCCESS)
    {
        return;
    }

    coCommandQueueSubmit(queue, &submitInfo, fence.get());

    coFenceWait(fence.get(), UINT64_MAX);
}


Coral::BufferPtr
createUniformBuffer(CoContext context, const UniformBlockBuilder& block)
{
    return createBuffer(context, block.data(), CO_BUFFER_TYPE_UNIFORM, /*cpuVisible*/ true);
}


Texture
createTexture(CoContext context, std::span<const uint8_t> buffer)
{
    auto img = loadImage(buffer);

    if (!img)
    {
        return {};
    }

    CoImageCreateConfig imageConfig
    {
        imageConfig.extent.width  = img->width,
        imageConfig.extent.height = img->height,
        imageConfig.hasMipMaps    = true,
        imageConfig.format        = img->format,
        imageConfig.usageHint     = CO_IMAGE_USAGE_HINT_SHADER_READ_ONLY,
    };

    Coral::ImagePtr image;
    if (coContextCreateImage(context, &imageConfig, std::out_ptr(image)) != CO_SUCCESS)
    {
        return {};
    }

    CoCommandQueue queue;
    if (coContextGetTransferQueue(context, &queue) != CO_SUCCESS)
    {
        return {};
    }

    CoCommandBufferCreateConfig commandBufferConfig{};
    Coral::CommandBufferPtr commandBuffer;
    if (coCommandQueueCreateCommandBuffer(queue, &commandBufferConfig, std::out_ptr(commandBuffer)) != CO_SUCCESS)
    {
        return {};
    }

    CoUpdateImageDataInfo updateInfo
    {
        updateInfo.image     = image.get(),
        updateInfo.pData     = reinterpret_cast<const CoByte*>(img->data.data()),
        updateInfo.dataCount = static_cast<uint32_t>(img->data.size()),
    };

    coCommandBufferBegin(commandBuffer.get());
    coCommandBufferUpdateImageData(commandBuffer.get(), &updateInfo);

    if (coImageGetMipLevelCount(image.get()) > 1)
    {
        coCommandBufferGenerateMipMaps(commandBuffer.get(), image.get());
    }

    coCommandBufferEnd(commandBuffer.get());
    auto cb = commandBuffer.get();
    CoCommandBufferSubmitInfo submitInfo
    {
        .pCommandBuffers    = &cb,
        .commandBufferCount = 1,
    };

    CoFenceCreateConfig fenceConfig{};
    Coral::FencePtr fence;
    if (coContextCreateFence(context, &fenceConfig, std::out_ptr(fence)) != CO_SUCCESS)
    {
        return {};
    }

    coCommandQueueSubmit(queue, &submitInfo, fence.get());
    coFenceWait(fence.get(), UINT64_MAX);

    CoSamplerCreateConfig samplerConfig
    {
        .minFilter    = CO_FILTER_LINEAR,
        .magFilter    = CO_FILTER_LINEAR,
        .mipmapFilter = CO_FILTER_LINEAR,
        .wrapMode     = CO_WRAP_MODE_CLAMP_TO_EDGE,
    };

    Coral::SamplerPtr sampler;
    if (coContextCreateSampler(context, &samplerConfig, std::out_ptr(sampler)) != CO_SUCCESS)
    {
        return {};
    }

    return { std::move(image), std::move(sampler) };
}

} // namespace Util
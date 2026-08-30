#include "DrawIndexed.hpp"

#include "Util.hpp"

#include <DefaultShader_slang.hpp>

#include <glm/gtc/matrix_inverse.hpp>

#include <map>

bool
DrawIndexed::initialize(CoContext context,
                        const Scene& scene)
{
    mContext      = context;
    mCameraParams = scene.cameraParams;
    mLightParams  = scene.lightParams;

    if (auto source = Util::compileShader(std::string(Shaders::DefaultShader_slang.begin(), Shaders::DefaultShader_slang.end()), "vertexShader"))
    {
        CoShaderModuleCreateConfig config
        {
            .pName       = "DefaultVertexShader",
            .stage       = CO_SHADER_STAGE_VERTEX,
            .pSource     = source->data(),
            .sourceCount = static_cast<uint32_t>(source->size()),
            .pEntryPoint = "main",
        };

        if (coContextCreateShaderModule(mContext, &config, std::out_ptr(mVertexShader)) != CO_SUCCESS)
        {
            return false;
        }
    }

    if (auto source = Util::compileShader(std::string(Shaders::DefaultShader_slang.begin(), Shaders::DefaultShader_slang.end()), "fragmentShader"))
    {
        CoShaderModuleCreateConfig config
        {
            .pName       = "DefaultFragmentShader",
            .stage       = CO_SHADER_STAGE_FRAGMENT,
            .pSource     = source->data(),
            .sourceCount = static_cast<uint32_t>(source->size()),
            .pEntryPoint = "main",
        };

        if (coContextCreateShaderModule(mContext, &config, std::out_ptr(mFragmentShader)) != CO_SUCCESS)
        {
            return false;
        }
    }

    std::vector<CoColorAttachmentInfo> colorAttachmentInfos{ { CO_PIXEL_FORMAT_RGBA8_SRGB, 0 } };
    CoDepthStencilAttachmentInfo depthStencilInfo{ CO_PIXEL_FORMAT_DEPTH24_STENCIL8 };

    CoPipelineStateCreateConfig pipelineStateConfig
    {
        .vertexShaderModule   = mVertexShader.get(),
        .fragmentShaderModule = mFragmentShader.get(),
        .framebufferLayout = {
            .pColorAttachments      = colorAttachmentInfos.data(),
            .colorAttachmentCount   = static_cast<uint32_t>(colorAttachmentInfos.size()),
            .depthStencilAttachment = &depthStencilInfo,
        },
        .faceCullingMode =
        {
            .cullMode    = CO_CULL_MODE_BACK,
            .orientation = CO_FRONT_FACE_ORIENTATION_CCW,
        },
        .depthTestMode = {
            .writeDepth = true,
            .compareOp  = CO_COMPARE_OP_LESS,
            .polygonOffset = {
                .factor = 0.f,
                .units  = 0.f,
            }
        },
        .blendMode = {
            .srcFactor  = CO_BLEND_FACTOR_ONE,
            .destFactor = CO_BLEND_FACTOR_ZERO,
            .blendOp    = CO_BLEND_OP_ADD,
        },
        .polygonMode = CO_POLYGON_MODE_SOLID,
        .topology    = CO_TOPOLOGY_TRIANGLE_LIST,
    };

    if (coContextCreatePipelineState(context, &pipelineStateConfig, std::out_ptr(mPipelineState)) != CO_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    if (!initializeBindings())
    {
        return false;
    }

    using Key   = std::pair<std::shared_ptr<const Util::Mesh>, std::shared_ptr<const Util::Material>>;
    using Value = std::vector<const RenderObject*>;

    std::map<Key, Value> map;

    for (const auto& ro : scene.renderObjects)
    {
        map[{ ro.mesh, ro.material }].push_back(&ro);
    }


    CoDescriptorLayout layout;
    coShaderModuleGetDescriptorLayout(mVertexShader.get(), &layout);
    std::span<const CoDescriptorInfo> descriptors(layout.pDescriptorInfos, layout.descriptorInfosCount);
    auto instanceParamsInfo = std::ranges::find_if(descriptors, [&](const auto& desc) { return desc.binding == mBindings.instanceParams; });

    for (const auto& [key, renderObjects] : map)
    {
        DrawBatch batch
        {
            .mesh     = key.first,
            .material = key.second,
        };

        for (const auto& r : renderObjects)
        {
            Util::UniformBlockBuilder builder(instanceParamsInfo->buffer);
            builder.set("instanceParams.modelMatrix", r->modelMatrix);
            builder.set("instanceParams.normalMatrix", glm::inverseTranspose(glm::mat3(r->modelMatrix)));
            batch.instanceParamsBuffers.push_back(Util::createUniformBuffer(mContext, builder));
        }
        mBatches.push_back(std::move(batch));
    }
    return true;
}


bool
DrawIndexed::initializeBindings()
{
    CoDescriptorLayout layout;
    coShaderModuleGetDescriptorLayout(mVertexShader.get(), &layout);
    std::span<const CoDescriptorInfo> vertexDescriptors(layout.pDescriptorInfos, layout.descriptorInfosCount);

    auto iter = std::ranges::find_if(vertexDescriptors, [](const auto& descriptor) { return descriptor.pName == std::string_view("cameraParams"); });
    if (iter == vertexDescriptors.end())
    {
        return false;
    }
    mBindings.cameraParams = iter->binding;

    iter = std::ranges::find_if(vertexDescriptors, [](const auto& descriptor) { return descriptor.pName == std::string_view("instanceParams"); });
    if (iter == vertexDescriptors.end())
    {
        return false;
    }
    mBindings.instanceParams = iter->binding;

    coShaderModuleGetDescriptorLayout(mFragmentShader.get(), &layout);
    std::span<const CoDescriptorInfo> fragmentDescriptors(layout.pDescriptorInfos, layout.descriptorInfosCount);

    iter = std::ranges::find_if(fragmentDescriptors, [](const auto& descriptor) { return descriptor.pName == std::string_view("lightParams"); });
    if (iter == fragmentDescriptors.end())
    {
        return false;
    }
    mBindings.lightParams = iter->binding;

    iter = std::ranges::find_if(fragmentDescriptors, [](const auto& descriptor) { return descriptor.pName == std::string_view("baseColorTexture"); });
    if (iter == fragmentDescriptors.end())
    {
        return false;
    }
    mBindings.baseColorTexture = iter->binding;

    return true;
}


void
DrawIndexed::drawOptimized(CoCommandBuffer commandBuffer)
{
    coCommandBufferBindPipeline(commandBuffer, mPipelineState.get());

    for (const auto& batch : mBatches)
    {
        coCommandBufferBindIndexBuffer(commandBuffer, batch.mesh->mIndexBuffer.get(), batch.mesh->mIndexFormat, 0);
        coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mPositionBuffer.get(), 0, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
        coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mNormalBuffer.get(), 1, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
        coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mTexcoordBuffer.get(), 2, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC2F));

        CoDescriptor descriptor = {
            .buffer = mCameraParams,
            .type   = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER
        };
        coCommandBufferBindDescriptor(commandBuffer, &descriptor, mBindings.cameraParams);

        descriptor = {
            .buffer = mLightParams,
            .type   = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER
        };
        coCommandBufferBindDescriptor(commandBuffer, &descriptor, mBindings.lightParams);

        descriptor =
        {
            .combinedImageSampler = {
                .image   = batch.material->mBaseColorTexture.mImage.get(),
                .sampler = batch.material->mBaseColorTexture.mSampler.get(),
            },
            .type  = CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        };
        coCommandBufferBindDescriptor(commandBuffer, &descriptor, mBindings.baseColorTexture);

        for (const auto& instanceParamsBuffer : batch.instanceParamsBuffers)
        {
            descriptor =
            {
                .buffer = instanceParamsBuffer.get(),
                .type   = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER
            };
            coCommandBufferBindDescriptor(commandBuffer, &descriptor, mBindings.instanceParams);

            CoDrawIndexedInfo drawInfo
            {
                .indexCount    = batch.mesh->mIndexCount,
                .instanceCount = 1,
                .firstIndex    = 0,
                .vertexOffset  = 0,
                .firstInstance = 0,
            };
            coCommandBufferDrawIndexed(commandBuffer, &drawInfo);
        }
    }
}


void
DrawIndexed::draw(CoCommandBuffer commandBuffer)
{
    coCommandBufferBindPipeline(commandBuffer, mPipelineState.get());

    for (const auto& batch : mBatches)
    {
        for (const auto& instanceParams : batch.instanceParamsBuffers)
        {
            CoDescriptor descriptor =
            {
                .buffer = mCameraParams,
                .type   = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            };
            coCommandBufferBindDescriptor(commandBuffer, &descriptor, mBindings.cameraParams);

            descriptor =
            {
                .buffer = instanceParams.get(),
                .type = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            };
            coCommandBufferBindDescriptor(commandBuffer, &descriptor, mBindings.instanceParams);

            descriptor =
            {
                .buffer = mLightParams,
                .type  = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER
            };
            coCommandBufferBindDescriptor(commandBuffer, &descriptor, mBindings.lightParams);

            descriptor =
            {
                .combinedImageSampler =
                {
                    .image   = batch.material->mBaseColorTexture.mImage.get(),
                    .sampler = batch.material->mBaseColorTexture.mSampler.get(),
                },
                .type = CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            };
            coCommandBufferBindDescriptor(commandBuffer, &descriptor, mBindings.baseColorTexture);

            coCommandBufferBindIndexBuffer(commandBuffer, batch.mesh->mIndexBuffer.get(), batch.mesh->mIndexFormat, 0);
            coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mPositionBuffer.get(), 0, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
            coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mNormalBuffer.get(), 1, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
            coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mTexcoordBuffer.get(), 2, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC2F));

            CoDrawIndexedInfo drawInfo
            {
                .indexCount    = batch.mesh->mIndexCount,
                .instanceCount = 1,
                .firstIndex    = 0,
                .vertexOffset  = 0,
                .firstInstance = 0,
            };

            coCommandBufferDrawIndexed(commandBuffer, &drawInfo);
        }
    }
}

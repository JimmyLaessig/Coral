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

    CoGraphicsPipelineCreateConfig GraphicsPipelineConfig
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

    if (coContextCreateGraphicsPipeline(context, &GraphicsPipelineConfig, std::out_ptr(mGraphicsPipeline)) != CO_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    using Key   = std::pair<std::shared_ptr<const Util::Mesh>, std::shared_ptr<const Util::Material>>;
    using Value = std::vector<const RenderObject*>;

    std::map<Key, Value> map;

    for (const auto& ro : scene.renderObjects)
    {
        map[{ ro.mesh, ro.material }].push_back(&ro);
    }

    for (const auto& [key, renderObjects] : map)
    {
        DrawBatch batch
        {
            .mesh     = key.first,
            .material = key.second,
        };

        for (const auto& r : renderObjects)
        {
            auto& instanceData = batch.instances.emplace_back();

            struct InstanceParams
            {
                glm::mat4 modelMatrix;
                glm::mat3x4 normalMatrix;
            };

            InstanceParams p
            {
                r->modelMatrix, 
                glm::inverseTranspose(glm::mat3(r->modelMatrix))
            };

            instanceData.instanceParamsBuffer = Util::createBuffer(context, std::span{ reinterpret_cast<const std::byte*>(&p), sizeof(p)}, CO_BUFFER_TYPE_UNIFORM);

            std::vector<CoDescriptorBinding> descriptorBindings =
            {
                CoDescriptorBinding
                {
                    .binding = 0,
                    .type    = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .buffer  = mCameraParams,
                },

                CoDescriptorBinding
                {
                    .binding = 1,
                    .type    = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .buffer  = instanceData.instanceParamsBuffer.get(),
                },
 
                CoDescriptorBinding
                {
                    .binding = 2,
                    .type    = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .buffer  = mLightParams,
                },
                CoDescriptorBinding
                {
                    .binding              = 3,
                    .type                 = CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .combinedImageSampler = {
                        .image   = batch.material->mBaseColorTexture.mImage.get(),
                        .sampler = batch.material->mBaseColorTexture.mSampler.get(),
                    }
                }
            };

            CoDescriptorSetCreateConfig descriptorSetConfig
            {
                .pDescriptorBindings    = descriptorBindings.data(),
                .descriptorBindingCount = static_cast<uint32_t>(descriptorBindings.size()),
            };

            if (coContextCreateDescriptorSet(mContext, &descriptorSetConfig, std::out_ptr(instanceData.descriptorSet)) != CO_SUCCESS)
            {
                return false;
            }
        }
        mBatches.push_back(std::move(batch));
    }
    return true;
}


void
DrawIndexed::drawOptimized(CoCommandBuffer commandBuffer)
{
    coCommandBufferBindPipeline(commandBuffer, mGraphicsPipeline.get());

    for (const auto& batch : mBatches)
    {
        coCommandBufferBindIndexBuffer(commandBuffer, batch.mesh->mIndexBuffer.get(), batch.mesh->mIndexFormat, 0);
        coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mPositionBuffer.get(), 0, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
        coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mNormalBuffer.get(), 1, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
        coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mTexcoordBuffer.get(), 2, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC2F));

        for (const auto& instanceData : batch.instances)
        {
            coCommandBufferBindDescriptorSet(commandBuffer, instanceData.descriptorSet.get(), 0);

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
    coCommandBufferBindPipeline(commandBuffer, mGraphicsPipeline.get());

    for (const auto& batch : mBatches)
    {
        for (const auto& instanceData : batch.instances)
        {
            coCommandBufferBindIndexBuffer(commandBuffer, batch.mesh->mIndexBuffer.get(), batch.mesh->mIndexFormat, 0);
            coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mPositionBuffer.get(), 0, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
            coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mNormalBuffer.get(), 1, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
            coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mTexcoordBuffer.get(), 2, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC2F));

            coCommandBufferBindDescriptorSet(commandBuffer, instanceData.descriptorSet.get(), 0);

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

#include "DrawInstanced.hpp"

#include "Util.hpp"

#include <InstancedShader_slang.hpp>

#include <glm/gtc/matrix_inverse.hpp>

#include <map>

struct InstanceParams
{
    glm::mat4 modelMatrix;
    glm::mat3x4 normalMatrix;
};

bool
DrawInstanced::initialize(CoContext context, const Scene& scene)
{
    mContext      = context;
    mCameraParams = scene.cameraParams;
    mLightParams  = scene.lightParams;

    if (auto source = Util::compileShader(std::string(Shaders::InstancedShader_slang.begin(), Shaders::InstancedShader_slang.end()), "vertexShader"))
    {
        CoShaderModuleCreateConfig config
        {
            .pName       = "InstancedVertexShader",
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

    if (auto source = Util::compileShader(std::string(Shaders::InstancedShader_slang.begin(), Shaders::InstancedShader_slang.end()), "fragmentShader"))
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

    CoGraphicsPipelineCreateConfig GraphicsPipelineConfig{};
    GraphicsPipelineConfig.vertexShaderModule   = mVertexShader.get();
    GraphicsPipelineConfig.fragmentShaderModule = mFragmentShader.get();

    GraphicsPipelineConfig.polygonMode                 = CO_POLYGON_MODE_SOLID;
    GraphicsPipelineConfig.topology                    = CO_TOPOLOGY_TRIANGLE_LIST;
    GraphicsPipelineConfig.faceCullingMode.cullMode    = CO_CULL_MODE_BACK;
    GraphicsPipelineConfig.faceCullingMode.orientation = CO_FRONT_FACE_ORIENTATION_CCW;

    GraphicsPipelineConfig.depthTestMode.writeDepth           = true;
    GraphicsPipelineConfig.depthTestMode.compareOp            = CO_COMPARE_OP_LESS;
    GraphicsPipelineConfig.depthTestMode.polygonOffset.factor = 0.f;
    GraphicsPipelineConfig.depthTestMode.polygonOffset.units  = 0.f;

    GraphicsPipelineConfig.blendMode.blendOp    = CO_BLEND_OP_ADD;
    GraphicsPipelineConfig.blendMode.srcFactor  = CO_BLEND_FACTOR_ONE;
    GraphicsPipelineConfig.blendMode.destFactor = CO_BLEND_FACTOR_ZERO;

    GraphicsPipelineConfig.framebufferLayout.pColorAttachments      = colorAttachmentInfos.data();
    GraphicsPipelineConfig.framebufferLayout.colorAttachmentCount   = static_cast<uint32_t>(colorAttachmentInfos.size());
    GraphicsPipelineConfig.framebufferLayout.depthStencilAttachment = &depthStencilInfo;

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

    for (const auto& [key, ro] : map)
    {
        DrawBatch batch
        {
            .mesh          = key.first,
            .material      = key.second,
            .instanceCount = static_cast<uint32_t>(ro.size()),
        };

        std::vector<InstanceParams> instanceParams;
        for (const auto& r : ro)
        {
            instanceParams.push_back({ r->modelMatrix, glm::inverseTranspose(glm::mat3(r->modelMatrix)) });
        }
        CoBufferCreateConfig bufferConfig
        {
            .size       = sizeof(InstanceParams) * instanceParams.size(),
            .type       = CO_BUFFER_TYPE_STORAGE,
            .cpuVisible = false
        };
        if (coContextCreateBuffer(mContext, &bufferConfig, std::out_ptr(batch.instanceParamsBuffer)) != CO_SUCCESS)
        {
            return false;
        }
        Util::updateBuffer(mContext, batch.instanceParamsBuffer.get(), std::as_bytes(std::span(instanceParams)));

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
                .type    = CO_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .buffer  = batch.instanceParamsBuffer.get(),
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

        if (coContextCreateDescriptorSet(mContext, &descriptorSetConfig, std::out_ptr(batch.descriptorSet)) != CO_SUCCESS)
        {
            return false;
        }

        mDrawBatches.push_back(std::move(batch));
    }

    return true;
}


void
DrawInstanced::draw(CoCommandBuffer commandBuffer)
{
    coCommandBufferBindPipeline(commandBuffer, mGraphicsPipeline.get());
    for (auto& batch : mDrawBatches)
    {
        coCommandBufferBindIndexBuffer(commandBuffer, batch.mesh ->mIndexBuffer.get(), batch.mesh->mIndexFormat, 0);
        coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mPositionBuffer.get(), 0, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
        coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mNormalBuffer.get(), 1, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
        coCommandBufferBindVertexBuffer(commandBuffer, batch.mesh->mTexcoordBuffer.get(), 2, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC2F));

        coCommandBufferBindDescriptorSet(commandBuffer, batch.descriptorSet.get(), 0);
        CoDrawIndexedInfo drawInfo
        {
            .indexCount    = batch.mesh->mIndexCount,
            .instanceCount = batch.instanceCount,
            .firstIndex    = 0,
            .vertexOffset  = 0,
            .firstInstance = 0,
        };

        coCommandBufferDrawIndexed(commandBuffer, &drawInfo);
    }
}

#include "DrawIndirect.hpp"

#include "Util.hpp"

#include <IndirectShader_slang.hpp>

#include <glm/gtc/matrix_inverse.hpp>

#include <map>

struct DrawParams
{
    uint64_t indexBufferAddress;
    uint64_t positionBufferAddress;
    uint64_t normalBufferAddress;
    uint64_t texcoordBufferAddress;
    uint32_t instanceIndex;
};

struct InstanceParams
{
    glm::mat4 modelMatrix;
    glm::mat3x4 normalMatrix;
};

bool
DrawIndirect::initialize(CoContext context,
                         const Scene& scene)
{
    mContext      = context;
    mCameraParams = scene.cameraParams;
    mLightParams  = scene.lightParams;

    if (auto source = Util::compileShader(std::string(Shaders::IndirectShader_slang.begin(), Shaders::IndirectShader_slang.end()), "vertexShader"))
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

    if (auto source = Util::compileShader(std::string(Shaders::IndirectShader_slang.begin(), Shaders::IndirectShader_slang.end()), "fragmentShader"))
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
        return false;
    }

    using Key   = std::shared_ptr<const Util::Material>;
    using Value = std::vector<const RenderObject*>;
    std::map< Key, Value> map;

    for (const auto& ro : scene.renderObjects)
    {
        map[{ ro.material }].push_back(&ro);
    }

    for (const auto& [key, ro] : map)
    {
        DrawBatch batch
        {
            .material  = key,
            .drawCount = static_cast<uint32_t>(ro.size()),
        };
        std::vector<DrawParams> drawParams;
        std::vector<InstanceParams> instanceParams;
        std::vector<CoDrawInfo> indirectBufferParams;
        for (const auto& r : ro)
        {
            drawParams.push_back({
                .indexBufferAddress    = coBufferGetAddress(r->mesh->mIndexBuffer.get()),
                .positionBufferAddress = coBufferGetAddress(r->mesh->mPositionBuffer.get()),
                .normalBufferAddress   = coBufferGetAddress(r->mesh->mNormalBuffer.get()),
                .texcoordBufferAddress = coBufferGetAddress(r->mesh->mTexcoordBuffer.get()),
                .instanceIndex         = static_cast<uint32_t>(instanceParams.size())
            });

            instanceParams.push_back({
                
                .modelMatrix  = r->modelMatrix,
                .normalMatrix = glm::inverseTranspose(glm::mat3(r->modelMatrix)),
            });

            indirectBufferParams.push_back({
                .vertexCount   = r->mesh->mIndexCount,
                .instanceCount = 1,
                .firstVertex   = 0,
                .firstInstance = 0,
            });
        }
        batch.indirectBuffer       = Util::createBuffer(mContext, std::as_bytes(std::span(indirectBufferParams)), CO_BUFFER_TYPE_INDIRECT);
        batch.drawParamsBuffer     = Util::createBuffer(mContext, std::as_bytes(std::span(drawParams)),           CO_BUFFER_TYPE_STORAGE);
        batch.instanceParamsBuffer = Util::createBuffer(mContext, std::as_bytes(std::span(instanceParams)),       CO_BUFFER_TYPE_STORAGE);

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
                .buffer  = batch.drawParamsBuffer.get(),
            },

            CoDescriptorBinding
            {
                .binding = 2,
                .type    = CO_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .buffer  = batch.instanceParamsBuffer.get(),
            },

            CoDescriptorBinding
            {
                .binding = 3,
                .type    = CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .buffer  = mLightParams,
            },
            CoDescriptorBinding
            {
                .binding              = 4,
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
DrawIndirect::draw(CoCommandBuffer commandBuffer)
{
    coCommandBufferBindPipeline(commandBuffer, mGraphicsPipeline.get());

    for (auto& batch : mDrawBatches)
    {
        coCommandBufferBindDescriptorSet(commandBuffer, batch.descriptorSet.get(), 0);
        coCommandBufferDrawIndirect(commandBuffer, batch.indirectBuffer.get(), 0, batch.drawCount, sizeof(CoDrawInfo));
    }
}

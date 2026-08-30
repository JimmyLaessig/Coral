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

    CoPipelineStateCreateConfig pipelineStateConfig{};
    pipelineStateConfig.vertexShaderModule   = mVertexShader.get();
    pipelineStateConfig.fragmentShaderModule = mFragmentShader.get();

    pipelineStateConfig.polygonMode                 = CO_POLYGON_MODE_SOLID;
    pipelineStateConfig.topology                    = CO_TOPOLOGY_TRIANGLE_LIST;
    pipelineStateConfig.faceCullingMode.cullMode    = CO_CULL_MODE_BACK;
    pipelineStateConfig.faceCullingMode.orientation = CO_FRONT_FACE_ORIENTATION_CCW;

    pipelineStateConfig.depthTestMode.writeDepth           = true;
    pipelineStateConfig.depthTestMode.compareOp            = CO_COMPARE_OP_LESS;
    pipelineStateConfig.depthTestMode.polygonOffset.factor = 0.f;
    pipelineStateConfig.depthTestMode.polygonOffset.units  = 0.f;

    pipelineStateConfig.blendMode.blendOp    = CO_BLEND_OP_ADD;
    pipelineStateConfig.blendMode.srcFactor  = CO_BLEND_FACTOR_ONE;
    pipelineStateConfig.blendMode.destFactor = CO_BLEND_FACTOR_ZERO;

    pipelineStateConfig.framebufferLayout.pColorAttachments      = colorAttachmentInfos.data();
    pipelineStateConfig.framebufferLayout.colorAttachmentCount   = static_cast<uint32_t>(colorAttachmentInfos.size());
    pipelineStateConfig.framebufferLayout.depthStencilAttachment = &depthStencilInfo;

    if (coContextCreatePipelineState(context, &pipelineStateConfig, std::out_ptr(mPipelineState)) != CO_SUCCESS)
    {
        return false;
    }

    if (!initializeBindings())
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

        mDrawBatches.push_back(std::move(batch));
    }

    return true;
}


bool
DrawIndirect::initializeBindings()
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

    iter = std::ranges::find_if(vertexDescriptors, [](const auto& descriptor) { return descriptor.pName == std::string_view("drawParams"); });
    if (iter == vertexDescriptors.end())
    {
        return false;
    }
    mBindings.drawParams = iter->binding;

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
DrawIndirect::draw(CoCommandBuffer commandBuffer)
{
    coCommandBufferBindPipeline(commandBuffer, mPipelineState.get());

    for (auto& batch : mDrawBatches)
    {
        CoDescriptor descriptor = {
            .buffer = batch.drawParamsBuffer.get(),
            .type   = CO_DESCRIPTOR_TYPE_STORAGE_BUFFER
        };

        coCommandBufferBindDescriptor(commandBuffer, &descriptor, mBindings.drawParams);

        descriptor = {
            .buffer = batch.instanceParamsBuffer.get(),
            .type   = CO_DESCRIPTOR_TYPE_STORAGE_BUFFER
        };

        coCommandBufferBindDescriptor(commandBuffer, &descriptor, mBindings.instanceParams);

        descriptor = {
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
            .type = CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        };
        coCommandBufferBindDescriptor(commandBuffer, &descriptor, mBindings.baseColorTexture);

        coCommandBufferDrawIndirect(commandBuffer, batch.indirectBuffer.get(), 0, batch.drawCount, sizeof(CoDrawInfo));
    }
}

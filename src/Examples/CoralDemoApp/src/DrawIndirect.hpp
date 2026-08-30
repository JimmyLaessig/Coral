#ifndef DRAWINDIRECT_HPP
#define DRAWINDIRECT_HPP

#include "Util.hpp"

#include "DrawInstanced.hpp"

#include <Coral/Util/RAII.hpp>
#include <Coral/Util/UniformBlockBuilder.hpp>

#include <glm/glm.hpp>

/*!
 *
 */
class DrawIndirect
{
public:

    bool initialize(CoContext context, 
                    const Scene& scene);

    void draw(CoCommandBuffer commandBuffer);

private:

    bool initializeBindings();

    CoContext mContext;

    CoBuffer mCameraParams;
    CoBuffer mLightParams;

    Coral::ShaderModulePtr mVertexShader;
    Coral::ShaderModulePtr mFragmentShader;
    Coral::PipelineStatePtr mPipelineState;

    struct
    {
        uint32_t cameraParams;
        uint32_t drawParams;
        uint32_t instanceParams;
        uint32_t lightParams;
        uint32_t baseColorTexture;
    } mBindings;

    struct DrawBatch
    {
        Coral::BufferPtr indirectBuffer;
        Coral::BufferPtr instanceParamsBuffer;
        Coral::BufferPtr drawParamsBuffer;
        std::shared_ptr<const Util::Material> material;
        uint32_t drawCount{ 0 };
    };

    std::vector<DrawBatch> mDrawBatches;

}; // class DrawIndirect

#endif // !DRAWINDIRECT_HPP

#ifndef DRAWINDEXED_HPP
#define DRAWINDEXED_HPP

#include "Util.hpp"

#include <Coral/Util/RAII.hpp>
#include <Coral/Util/UniformBlockBuilder.hpp>

#include <glm/glm.hpp>

struct RenderObject
{
    std::shared_ptr<const Util::Mesh> mesh;
    std::shared_ptr<const Util::Material> material;
    glm::mat4 modelMatrix;
}; // struct RenderObject

struct Scene
{
    const std::vector<RenderObject>& renderObjects;
    CoBuffer cameraParams;
    CoBuffer lightParams;
};

/*!
 *
 */
class DrawIndexed
{
public:

    bool initialize(CoContext context, const Scene& scene);

    void draw(CoCommandBuffer commandBuffer);

    void drawOptimized(CoCommandBuffer commandBuffer);

private:
    
    bool initializeBindings();

    CoContext mContext;

    Coral::ShaderModulePtr mVertexShader;
    Coral::ShaderModulePtr mFragmentShader;
    Coral::PipelineStatePtr mPipelineState;

    CoBuffer mCameraParams;
    CoBuffer mLightParams;

    struct DrawBatch
    {
        std::shared_ptr<const Util::Mesh> mesh;
        std::shared_ptr<const Util::Material> material;
        std::vector<Coral::BufferPtr> instanceParamsBuffers;
    };

    std::vector<DrawBatch> mBatches;

    struct
    {
        uint32_t cameraParams;
        uint32_t instanceParams;
        uint32_t lightParams;
        uint32_t baseColorTexture;
    } mBindings;


}; // class DrawIndexed

#endif // !DRAWINDEXED_HPP

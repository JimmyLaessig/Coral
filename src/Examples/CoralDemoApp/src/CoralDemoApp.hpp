#ifndef CORALDEMOAPP_HPP
#define CORALDEMOAPP_HPP

#include "FlyCamera.hpp"

#include "DrawIndexed.hpp"
#include "DrawInstanced.hpp"
#include "DrawIndirect.hpp"

#include <GLFW/glfw3.h>

#include <Coral/Util/RAII.hpp>
#include <Coral/Util/UniformBlockBuilder.hpp>

#include <glm/glm.hpp>

enum class DrawMode
{
    INDEXED           = 0,
    INDEXED_OPTIMIZED = 1,
    INSTANCED         = 2,
    INDIRECT          = 3,
};



class CoralDemoApp
{
public:

    ~CoralDemoApp();

    bool initialize();

    int run();

private:

    void renderUI(float deltaT);

    GLFWwindow* mWindow;

    Coral::ContextPtr mContext;

    Coral::SwapchainPtr mSwapchain;

    std::shared_ptr<Util::Mesh> mMesh;
    std::shared_ptr<Util::Material> mMaterial;

    Util::UniformBlockBuilder mCameraParams;
    std::shared_ptr<CoBuffer_T> mCameraParamsBuffer;

    Util::UniformBlockBuilder mLightParams;
    std::shared_ptr<CoBuffer_T> mLightParamsBuffer;

    std::vector<RenderObject> mRenderObjects;

    FlyCamera mCamera;

    float mTimeSinceLastFrameTimeUpdate{ 0.f };
    float mDisplayedFrameTime{ 0.f };

    float mRotationsPerSecond{ 0.f };
    float mRotation{ 0.f };

    DrawMode mDrawMode{ DrawMode::INDEXED };

    DrawIndexed mDrawIndexed;
    DrawInstanced mDrawInstanced;
    DrawIndirect mDrawIndirect;

}; // class CoralDemoApp

#endif // !CORALDEMOAPP_HPP

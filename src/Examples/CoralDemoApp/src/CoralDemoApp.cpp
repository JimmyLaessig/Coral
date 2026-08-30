#include "CoralDemoApp.hpp"

#include "Util.hpp"

#include <Coral/Resources/Cube.hpp>
#include <Coral/Resources/Sphere.hpp>
#include <Coral/Resources/uvtest_png.hpp>



#define GLFW_EXPOSE_NATIVE_WIN32
#pragma warning( push )
#pragma warning (disable: 4005)
#include <GLFW/glfw3native.h>
#pragma warning( pop )
#undef DELETE
#undef MAX

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <Coral/Coral.h>
#include <Coral/ImGui_Impl_Coral.h>

#include <Coral/Util/RAII.hpp>
#include <Coral/Util/UniformBlockBuilder.hpp>

#include <backends/imgui_impl_glfw.h>

#include <array>
#include <chrono>
#include <memory>
#include <span>
#include <vector>
#include <ranges>

CoralDemoApp::~CoralDemoApp()
{
    ImGui_ImplCoral_Shutdown(mContext.get());
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


bool
CoralDemoApp::initialize()
{
    if (glfwInit() != GLFW_TRUE)
    {
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const auto WIDTH = 1024;
    const auto HEIGHT = 768;

    mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Coral Demo Application", nullptr, nullptr);

    glfwShowWindow(mWindow);

    CoContextCreateConfig contextConfig
    {
        .graphicsAPI      = CO_GRAPHICS_API_VULKAN,
        .pApplicationName = "Coral Demo Application",
    };
    

    if (coCreateContext(&contextConfig, std::out_ptr(mContext)) != CO_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    auto depthFormat = CO_PIXEL_FORMAT_DEPTH24_STENCIL8;

    CoSwapchainCreateConfig swapchainConfig
    {
        .nativeWindowHandle = glfwGetWin32Window(mWindow),
        .format             = CO_PIXEL_FORMAT_RGBA8_SRGB,
        .depthFormat        = &depthFormat,
        .minImageCount      = 2,
    };

    if (coContextCreateSwapchain(mContext.get(), &swapchainConfig, std::out_ptr(mSwapchain)) != CO_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    auto swapchainImageCount = coSwapchainGetImageCount(mSwapchain.get());

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplGlfw_InitForVulkan(mWindow, true);

    std::vector<CoColorAttachmentInfo> colorAttachmentInfos{ { CO_PIXEL_FORMAT_RGBA8_SRGB, 0 } };
    CoDepthStencilAttachmentInfo depthStencilInfo{ CO_PIXEL_FORMAT_DEPTH24_STENCIL8 };

    ImGui_ImplCoral_InitInfo initInfo
    {
        .context                    = mContext.get(),
        .framebufferLayout = {
            .pColorAttachments      = colorAttachmentInfos.data(),
            .colorAttachmentCount   = static_cast<uint32_t>(colorAttachmentInfos.size()),
            .depthStencilAttachment = &depthStencilInfo,
         },
        .swapchainImageCount        = swapchainImageCount,
    };

    if (ImGui_ImplCoral_Init(&initInfo) != CO_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    if (ImGui_ImplCoral_CreateFontsTexture(mContext.get()) != CO_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    Cube cube;
    auto cubeMesh             = std::make_shared<Util::Mesh>();
    cubeMesh->mPositionBuffer = Util::createBuffer(mContext.get(), std::as_bytes(std::span(cube.positions)), CO_BUFFER_TYPE_VERTEX);
    cubeMesh->mNormalBuffer   = Util::createBuffer(mContext.get(), std::as_bytes(std::span(cube.normals)), CO_BUFFER_TYPE_VERTEX);
    cubeMesh->mTexcoordBuffer = Util::createBuffer(mContext.get(), std::as_bytes(std::span(cube.texcoords)), CO_BUFFER_TYPE_VERTEX);
    cubeMesh->mIndexBuffer    = Util::createBuffer(mContext.get(), std::as_bytes(std::span(cube.indices)), CO_BUFFER_TYPE_INDEX);
    cubeMesh->mIndexFormat    = CO_INDEX_FORMAT_UINT16;
    cubeMesh->mIndexCount     = static_cast<uint32_t>(cube.indices.size());

    Sphere sphere = Sphere::Generate();
    auto sphereMesh = std::make_shared<Util::Mesh>();
    sphereMesh->mPositionBuffer = Util::createBuffer(mContext.get(), std::as_bytes(std::span(sphere.positions)), CO_BUFFER_TYPE_VERTEX);
    sphereMesh->mNormalBuffer   = Util::createBuffer(mContext.get(), std::as_bytes(std::span(sphere.normals)), CO_BUFFER_TYPE_VERTEX);
    sphereMesh->mTexcoordBuffer = Util::createBuffer(mContext.get(), std::as_bytes(std::span(sphere.texcoords)), CO_BUFFER_TYPE_VERTEX);
    sphereMesh->mIndexBuffer    = Util::createBuffer(mContext.get(), std::as_bytes(std::span(sphere.indices)), CO_BUFFER_TYPE_INDEX);
    sphereMesh->mIndexFormat    = CO_INDEX_FORMAT_UINT16;
    sphereMesh->mIndexCount     = static_cast<uint32_t>(sphere.indices.size());

    mMaterial = std::make_shared<Util::Material>();
    mMaterial->mBaseColorTexture = Util::createTexture(mContext.get(), Coral::Resources::uvtest_png);

    mRenderObjects.reserve(100 * 100);
    for (int i = 0; i < 100; ++i)
    {
        for (int j = 0; j < 100; ++j)
        {
            auto mesh        = mRenderObjects.size() % 2 == 0 ? cubeMesh : sphereMesh;
            auto modelMatrix = glm::translate(glm::vec3((i - 50) * 2, 0.f, (j - 50) * 2));
            mRenderObjects.push_back({ 
                .mesh        = mesh, 
                .material    = mMaterial, 
                .modelMatrix = modelMatrix
            });
        }
    }

    {
        std::array<CoStructMemberInfo, 1> members
        {
            CoStructMemberInfo{
                .pName = "viewProjectionMatrix",
                .count = 1,
                .offset = 0,
                .stride = 64,
                .type = CO_STRUCT_MEMBER_TYPE_MATRIX,
                .matrix = {
                    .dataType    = CO_SCALAR_TYPE_FLOAT32,
                    .rowCount    = 4,
                    .columnCount = 4,
                    .stride      = 16,
                }
            },
        };

        CoBufferInfo cameraParamsDef
        {
            .structure = {
                .pName  = "cameraParams",
                .count  = 1,
                .offset = 0,
                .stride = 64,
                .type   = CO_STRUCT_MEMBER_TYPE_STRUCT,
                .structure = {
                    .pTypeName   = "CameraParams",
                    .pMembers    = members.data(),
                    .memberCount = static_cast<uint32_t>(members.size()),
                },
            },
        };
        mCameraParams       = { cameraParamsDef };
        mCameraParamsBuffer = Util::createUniformBuffer(mContext.get(), mCameraParams);
    }

    {
        std::array<CoStructMemberInfo, 3> members
        {
            CoStructMemberInfo{
                .pName  = "lightDirection",
                .count  = 1,
                .offset = 0,
                .stride = 16,
                .type   = CO_STRUCT_MEMBER_TYPE_VECTOR,
                .vector = {
                    .dataType       = CO_SCALAR_TYPE_FLOAT32,
                    .componentCount = 3
                }
            },
            CoStructMemberInfo{
                .pName  = "lightColor",
                .count  = 1,
                .offset = 16,
                .stride = 16,
                .type = CO_STRUCT_MEMBER_TYPE_VECTOR,
                .vector = {
                    .dataType       = CO_SCALAR_TYPE_FLOAT32,
                    .componentCount = 3
                }
            },
            CoStructMemberInfo{
                .pName  = "ambientColor",
                .count  = 1,
                .offset = 32,
                .stride = 16,
                .type   = CO_STRUCT_MEMBER_TYPE_VECTOR,
                .vector = {
                    .dataType       = CO_SCALAR_TYPE_FLOAT32,
                    .componentCount = 3
                }
            },
        };

        CoBufferInfo lightParamsBufferInfo
        {
            .structure  = {
                .pName  = "lightParams",
                .count  = 1,
                .offset = 0,
                .stride = 48,
                .type   = CO_STRUCT_MEMBER_TYPE_STRUCT,
                .structure = {
                    .pTypeName   = "LightParams",
                    .pMembers    = members.data(),
                    .memberCount = static_cast<uint32_t>(members.size()),
                },
            }
        };

        mLightParams = { lightParamsBufferInfo };
        mLightParams.set("lightParams.lightColor",     glm::vec3(1.f, 1.f, 1.f));
        mLightParams.set("lightParams.lightDirection", glm::normalize(glm::vec3(1.f, 2.f, 0.f)));
        mLightParams.set("lightParams.ambientColor",   glm::vec3(0.1f, 0.1f, 0.1f));
        mLightParamsBuffer = Util::createUniformBuffer(mContext.get(), mLightParams);
    }
    Scene scene
    {
        .renderObjects = mRenderObjects,
        .cameraParams  = mCameraParamsBuffer.get(),
        .lightParams   = mLightParamsBuffer.get()
    };
    return mDrawIndexed.initialize(mContext.get(), scene) &&
           mDrawInstanced.initialize(mContext.get(), scene) &&
           mDrawIndirect.initialize(mContext.get(), scene);
}


void
CoralDemoApp::renderUI(float deltaT)
{
    float displayedFrameTimeUpdateInterval = 1.f;

    mTimeSinceLastFrameTimeUpdate += deltaT;
    if (mTimeSinceLastFrameTimeUpdate > displayedFrameTimeUpdateInterval)
    {
        mTimeSinceLastFrameTimeUpdate -= displayedFrameTimeUpdateInterval;
        mDisplayedFrameTime = deltaT;
    }

    auto& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10, 10), 0, ImVec2(1, 0));
    if (ImGui::Begin("Perf. Overlay", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing))
    {
        if (ImGui::BeginTable("Perf. Overlay Table", 3, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("FPS");

            ImGui::TableNextColumn();
            ImGui::Text(":");

            ImGui::TableNextColumn();
            ImGui::Text("%d", static_cast<int>(1.f / mDisplayedFrameTime));

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("Frame time (ms)");

            ImGui::TableNextColumn();
            ImGui::Text(":");

            ImGui::TableNextColumn();
            ImGui::Text("%.2f", mDisplayedFrameTime * 1000);

            ImGui::EndTable();

            ImGui::Text("Rotation speed");
            ImGui::PushID("rotationPerSecond");
            ImGui::SliderFloat("", &mRotationsPerSecond, -1.f, 1.f);
            ImGui::PopID();

            std::array<const char*, 4> drawModeOptions{ "INDEXED", "INDEXED_OPTIMIZED", "INSTANCED", "INDIRECT" };
            int selected = static_cast<int>(mDrawMode);
            if (ImGui::Combo("Draw mode", &selected, drawModeOptions.data(), drawModeOptions.size()))
            {
                mDrawMode = static_cast<DrawMode>(selected);
            }
        }
    }
}


int
CoralDemoApp::run()
{
    uint32_t currentFrameIndex = 0;

    auto swapchainImageCount = coSwapchainGetImageCount(mSwapchain.get());

    std::vector<Coral::SemaphorePtr> imageReadySemaphores(swapchainImageCount);
    std::vector<Coral::SemaphorePtr> renderFinishedSemaphores(swapchainImageCount);
    std::vector<Coral::FencePtr> currentFrameFences(swapchainImageCount);

    CoCommandQueue queue;
    coContextGetGraphicsQueue(mContext.get(), &queue);

    for (const auto& [imageReadySemaphore,
                      renderFinishedSemaphore,
                      currentFrameFence] : std::views::zip(imageReadySemaphores,
                                                           renderFinishedSemaphores,
                                                           currentFrameFences))
    {
        CoSemaphoreCreateConfig semaphoreConfig{};
        if (coContextCreateSemaphore(mContext.get(), &semaphoreConfig, std::out_ptr(imageReadySemaphore)) != CO_SUCCESS)
        {
            return EXIT_FAILURE;
        }

        if (coContextCreateSemaphore(mContext.get(), &semaphoreConfig, std::out_ptr(renderFinishedSemaphore)) != CO_SUCCESS)
        {
            return EXIT_FAILURE;
        }

        CoFenceCreateConfig fenceConfig{};
        fenceConfig.createSignaled = true;
        if (coContextCreateFence(mContext.get(), &fenceConfig, std::out_ptr(currentFrameFence)) != CO_SUCCESS)
        {
            return EXIT_FAILURE;
        }
    }

    auto before = std::chrono::system_clock::now();

    auto fov       = glm::radians(65.f);
    auto nearPlane = 0.01f;
    auto farPlane  = 1000.f;

    int width, height;
    glfwGetWindowSize(mWindow, &width, &height);
    auto projectionMatrix = glm::perspective(fov, static_cast<float>(width) / height, nearPlane, farPlane);

    ImVec2 prevMousePos = ImGui::GetMousePos();
   
    // Start the game loop
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();

        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
        }

        int width, height;
        glfwGetWindowSize(mWindow, &width, &height);

        auto imageReadySemaphore     = imageReadySemaphores[currentFrameIndex].get();
        auto renderFinishedSemaphore = renderFinishedSemaphores[currentFrameIndex].get();

        CoAcquiredImageInfo info{};
        if (coSwapchainAcquireNextImage(mSwapchain.get(),
                                        imageReadySemaphore,
                                        nullptr,
                                        &info) != CO_SUCCESS)
        {
            return EXIT_FAILURE;
        }

        auto mousePos   = ImGui::GetMousePos();
        auto mouseDelta = glm::vec2(mousePos.x - prevMousePos.x, 
                                    mousePos.y - prevMousePos.y);
        prevMousePos    = mousePos;
        
        auto now        = std::chrono::system_clock::now();
        float deltaT    = std::chrono::duration_cast<std::chrono::nanoseconds>(now - before).count() / 1e+9f;
        before          = now;
 
        mCamera.update(ImGui::IsKeyDown(ImGuiKey_W),
                       ImGui::IsKeyDown(ImGuiKey_S),
                       ImGui::IsKeyDown(ImGuiKey_A),
                       ImGui::IsKeyDown(ImGuiKey_D),
                       deltaT,
                       ImGui::IsMouseDown(ImGuiMouseButton_Right) * mouseDelta.x / width,
                       ImGui::IsMouseDown(ImGuiMouseButton_Right) * mouseDelta.y / height);

        renderUI(deltaT);

        ImGui::End();

        ImGui::Render();

        mRotation += (mRotationsPerSecond * 360.f) * deltaT;

        // Update the camera parameters
        auto viewMatrix       = mCamera.viewMatrix();
        auto projectionMatrix = glm::perspective(fov, static_cast<float>(width) / height, nearPlane, farPlane);

        mCameraParams.set("cameraParams.viewProjectionMatrix", projectionMatrix * viewMatrix);
        Util::updateBuffer(mContext.get(), mCameraParamsBuffer.get(), mCameraParams.data());

        CoCommandBufferCreateConfig commandBufferConfig{};
        Coral::CommandBufferPtr commandBuffer;
        if (coCommandQueueCreateCommandBuffer(queue, &commandBufferConfig, std::out_ptr(commandBuffer)) != CO_SUCCESS)
        {
            return EXIT_FAILURE;
        }

        coCommandBufferBegin(commandBuffer.get());

        CoClearColor clearColor{ 0, CO_CLEAR_OP_CLEAR, { 1.f, 1.f, 1.f, 1.f } };
        CoClearDepthStencil clearDepth{ CO_CLEAR_OP_CLEAR, 1.f, 0 };

        CoBeginRenderPassInfo beginRenderPassInfo
        {
            .framebuffer       = info.framebuffer,
            .pClearColors      = &clearColor,
            .clearColorsCount  = 1,
            .clearDepthStencil = &clearDepth,
        };

        coCommandBufferBeginRenderPass(commandBuffer.get(), &beginRenderPassInfo);

        CoViewportInfo viewport
        {
            .viewport = CoRectangle{ 0, 0, static_cast<uint32_t>(width),  static_cast<uint32_t>(height) },
            .minDepth = 0.f,
            .maxDepth = 1.f,
        };

        coCommandBufferSetViewport(commandBuffer.get(), &viewport);

        if (mDrawMode == DrawMode::INDEXED)
        {
            mDrawIndexed.draw(commandBuffer.get());
        }
        if (mDrawMode == DrawMode::INDEXED_OPTIMIZED)
        {
            mDrawIndexed.drawOptimized(commandBuffer.get());
        }
        if (mDrawMode == DrawMode::INSTANCED)
        {
            mDrawInstanced.draw(commandBuffer.get());
        }
        if (mDrawMode == DrawMode::INDIRECT)
        {
            mDrawIndirect.draw(commandBuffer.get());
        }

        ImGui_ImplCoral_NewFrame(mContext.get());

        ImGui_ImplCoral_RenderDrawData(mContext.get(), ImGui::GetDrawData(), commandBuffer.get());

        coCommandBufferEndRenderPass(commandBuffer.get());
        coCommandBufferEnd(commandBuffer.get());

        CoCommandBufferSubmitInfo submitInfo{};
        auto cb                       = commandBuffer.get();
        submitInfo.pCommandBuffers    = &cb;
        submitInfo.commandBufferCount = 1;
        submitInfo.pWaitSemaphores    = &imageReadySemaphore;
        submitInfo.waitSemaphoreCount = 1;

        submitInfo.pSignalSemaphores    = &renderFinishedSemaphore;
        submitInfo.signalSemaphoreCount = 1;

        if (coCommandQueueSubmit(queue, &submitInfo, nullptr) != CO_SUCCESS)
        {
            return EXIT_FAILURE;
        }

        CoPresentInfo presentInfo{};
        presentInfo.swapchain          = mSwapchain.get();
        presentInfo.pWaitSemaphores    = &renderFinishedSemaphore;
        presentInfo.waitSemaphoreCount = 1;

        if (coCommandQueuePresent(queue, &presentInfo) != CO_SUCCESS)
        {
            return EXIT_FAILURE;
        }

        if (coCommandQueueWaitIdle(queue) != CO_SUCCESS)
        {
            return EXIT_FAILURE;
        }

        currentFrameIndex = (currentFrameIndex + 1) % coSwapchainGetImageCount(mSwapchain.get());
    }

    return 0;
}
#include "Cube.hpp"
#include "TexturedWithLightingShader.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#pragma warning( push )
#pragma warning (disable: 4005)
#include <GLFW/glfw3native.h>
#pragma warning( pop )
#undef DELETE
#undef MAX

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <array>
#include <chrono>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include <Coral/Coral.hpp>
#include <Coral/UniformBlockBuilder.hpp>
#include <Coral/ShaderLanguage.hpp>
#include <Coral/ShaderGraph/CompilerSPV.hpp>

#include <Coral/ImGui_Impl_Coral.hpp>
#include <backends/imgui_impl_glfw.h>

namespace
{

struct Image
{
	std::vector<std::byte> data;

	uint32_t width{ 0 };
	uint32_t height{ 0 };

	Coral::PixelFormat format{ Coral::PixelFormat::RGBA8_UI };
};

bool loadImage(const char* path, Image& image)
{
	int x, y, c;
	stbi__vertically_flip_on_load = true;
	auto data = stbi_load(path, &x, &y, &c, 0);

	if (data == nullptr)
	{
		return false;
	}

	image.width		= static_cast<uint32_t>(x);
	image.height	= static_cast<uint32_t>(y);
	image.data.resize(x * y * c);
	std::memcpy(image.data.data(), data, image.data.size());

	stbi_image_free(data);

	switch (1)
	{
	case 1:
		switch (c)
		{
			case 1:	image.format = Coral::PixelFormat::R8_UI;		break;
			case 2:	image.format = Coral::PixelFormat::RG8_UI;		break;
			case 3:	image.format = Coral::PixelFormat::RGB8_UI;		break;
			case 4:	image.format = Coral::PixelFormat::RGBA8_UI;	break;
			default:
				return false;
		}
		break;
	case 2:
		switch (c)
		{
			case 1:	image.format = Coral::PixelFormat::R16_UI;		break;
			case 2:	image.format = Coral::PixelFormat::RG16_UI;		break;
			case 3:	image.format = Coral::PixelFormat::RGB16_UI;	break;
			case 4:	image.format = Coral::PixelFormat::RGBA16_UI;	break;
			default:
				return false;
		}
		break;
	}

	return true;
}

template<typename T>
Coral::AttributeFormat toAttributeFormat();

template<> Coral::AttributeFormat toAttributeFormat<glm::vec3>() { return Coral::AttributeFormat::VEC3F; }
template<> Coral::AttributeFormat toAttributeFormat<glm::vec2>() { return Coral::AttributeFormat::VEC2F; }
template<> Coral::AttributeFormat toAttributeFormat<uint32_t>()  { return Coral::AttributeFormat::UINT32; }
template<> Coral::AttributeFormat toAttributeFormat<uint16_t>()  { return Coral::AttributeFormat::UINT16; }

template<typename T, size_t S>
std::pair<Coral::BufferPtr, Coral::BufferViewPtr>
createBuffer(Coral::Context* context, const std::array<T, S>& elements, Coral::BufferType type)
{
	Coral::BufferCreateConfig bufferConfig{};
	bufferConfig.size       = elements.size() * sizeof(T);
	bufferConfig.type       = type;
	bufferConfig.cpuVisible = false;
	Coral::BufferPtr buffer(context->createBuffer(bufferConfig).value());

	Coral::BufferViewCreateConfig bufferViewConfig{};
	bufferViewConfig.buffer      = buffer.get();
	bufferViewConfig.count		 = static_cast<uint32_t>(elements.size());
	bufferViewConfig.offset		 = 0;
	bufferViewConfig.stride		 = 0;
	bufferViewConfig.attribute   = toAttributeFormat<T>();
	Coral::BufferViewPtr bufferView(context->createBufferView(bufferViewConfig).value());

	auto queue = context->getTransferQueue();

	Coral::CommandBufferCreateConfig commandBufferConfig{};
	Coral::CommandBufferPtr commandBuffer(queue->createCommandBuffer(commandBufferConfig).value());

	Coral::UpdateBufferDataInfo updateInfo{};
	updateInfo.buffer = buffer.get();
	updateInfo.offset = 0;
	updateInfo.data   = std::as_bytes(std::span(elements));
	
	commandBuffer->begin();
	commandBuffer->cmdUpdateBufferData(updateInfo);
	commandBuffer->end();

	auto commandBufferPtr = commandBuffer.get();

	Coral::CommandBufferSubmitInfo submitInfo{};
	submitInfo.commandBuffers = { &commandBufferPtr , 1 };

	auto fence = context->createFence().value();
	queue->submit(submitInfo, fence.get());
	fence->wait();

	return { std::move(buffer), std::move(bufferView) };
}


void
updateUniformBuffer(Coral::Buffer& buffer, Coral::UniformBlockBuilder& block)
{
	auto mapped = buffer.map();
	auto data = block.data();
	std::memcpy(mapped, data.data(), data.size());

	buffer.unmap();
}


Coral::BufferPtr
createUniformBuffer(Coral::Context& context, Coral::UniformBlockBuilder& block)
{
	Coral::BufferCreateConfig config{};
	config.cpuVisible	= true;
	config.size			= block.data().size();
	config.type			= Coral::BufferType::UNIFORM_BUFFER;

	Coral::BufferPtr buffer(context.createBuffer(config).value());

	updateUniformBuffer(*buffer, block);

	return std::move(buffer);
}


std::pair<Coral::ImagePtr, Coral::SamplerPtr>
createTexture(Coral::Context& context, const std::string& path)
{
	Image img;
	if (!loadImage(path.c_str(), img))
	{
		return {};
	}

	Coral::ImageCreateConfig imageConfig{};
	imageConfig.width	= img.width;
	imageConfig.height	= img.height;
	imageConfig.format	= img.format;
	imageConfig.hasMips = true;
	auto image          = context.createImage(imageConfig).value();

	auto queue = context.getTransferQueue();

	Coral::CommandBufferCreateConfig commandBufferConfig{};

	auto commandBuffer = queue->createCommandBuffer(commandBufferConfig).value();

	Coral::UpdateImageDataInfo updateInfo{};
	updateInfo.image	  = image.get();
	updateInfo.updateMips = image->getMipLevels() > 1;
	updateInfo.data		  = { reinterpret_cast<const std::byte*>(img.data.data()), img.data.size() };

	commandBuffer->begin();
	commandBuffer->cmdUpdateImageData(updateInfo);
	commandBuffer->end();

	Coral::CommandBuffer* commandBufferPtr = commandBuffer.get();

	Coral::CommandBufferSubmitInfo submitInfo{};
	submitInfo.commandBuffers = { &commandBufferPtr , 1 };

	auto fence = context.createFence().value();
	queue->submit(submitInfo, fence.get());
	fence->wait();

	Coral::SamplerCreateConfig samplerConfig{};
	samplerConfig.wrapMode = Coral::WrapMode::REPEAT;
	auto sampler           = context.createSampler(samplerConfig).value();

	return std::pair<Coral::ImagePtr, Coral::SamplerPtr>{ std::move(image), std::move(sampler) };
}

} // namespace


int main()
{
	if (glfwInit() != GLFW_TRUE)
	{
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	const auto WIDTH = 1024;
	const auto HEIGHT = 768;

	auto window = glfwCreateWindow(WIDTH, HEIGHT, "Coral Demo Application", nullptr, nullptr);

	glfwShowWindow(window);

	Coral::SwapchainCreateConfig swapchainConfig{};
	swapchainConfig.nativeWindowHandle  = glfwGetWin32Window(window);
	swapchainConfig.depthFormat		    = Coral::PixelFormat::DEPTH24_STENCIL8;
	swapchainConfig.format			    = Coral::PixelFormat::RGBA8_SRGB;
	swapchainConfig.swapchainImageCount = 2;

	Coral::ContextCreateConfig contextConfig{};
	contextConfig.graphicsAPI = Coral::GraphicsAPI::VULKAN;
	auto context			  = Coral::createContext(contextConfig).value();

	auto swapchain = context->createSwapchain(swapchainConfig).value();
	auto fence     = context->createFence().value();
	auto queue     = context->getGraphicsQueue();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

	ImGui_ImplGlfw_InitForVulkan(window, true);

	ImGui_ImplCoral_InitInfo initInfo{};
	initInfo.context   = context.get();
	initInfo.swapchain = swapchain.get();
	ImGui_ImplCoral_Init(&initInfo);

	auto shader = TexturedWithLightingShader::shaderSource();

	if (!shader)
	{
		return EXIT_FAILURE;
	}

	Coral::ShaderModuleCreateConfig vertexShaderConfig{};
	vertexShaderConfig.name		  = "VertexShader";
	vertexShaderConfig.stage	  = Coral::ShaderStage::VERTEX;
	vertexShaderConfig.entryPoint = "main";
	vertexShaderConfig.source	  = std::as_bytes(std::span{ shader->vertexShader });
	auto vertexShader             = context->createShaderModule(vertexShaderConfig).value();

	Coral::ShaderModuleCreateConfig fragmentShaderConfig{};
	fragmentShaderConfig.name		= "FragmentShader";
	fragmentShaderConfig.stage		= Coral::ShaderStage::FRAGMENT;
	fragmentShaderConfig.entryPoint	= "main";
	fragmentShaderConfig.source		= std::as_bytes(std::span{ shader->fragmentShader });
	auto fragmentShader             = context->createShaderModule(fragmentShaderConfig).value();

	glm::mat4 modelMatrix(1.f);

	auto viewMatrix       = glm::lookAt(glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	auto fov			  = glm::radians(65.f);
	auto nearPlane		  = 0.01f;
	auto farPlane		  = 1000.f;
	auto projectionMatrix = glm::perspective(fov, static_cast<float>(WIDTH) / HEIGHT, nearPlane, farPlane);

	Coral::UniformBlockBuilder uniformBlock(TexturedWithLightingShader::uniformBlockDefinition());
	uniformBlock.setVec3F("lightColor", glm::vec3{ 1.f, 1.f, 1.f });
	uniformBlock.setVec3F("lightDirection", glm::normalize(glm::vec3{ 1.f, 1.f, 1.f }));
	uniformBlock.setMat44F("modelViewProjectionMatrix", projectionMatrix * viewMatrix * modelMatrix);
	uniformBlock.setMat33F("normalMatrix", glm::mat3(glm::transpose(glm::inverse(modelMatrix))));

	auto uniformBuffer = createUniformBuffer(*context, uniformBlock);

	auto [texture, sampler] = createTexture(*context, "resources/uvtest.png");

	auto [positionBuffer, positionBufferView] = ::createBuffer(context.get(), Cube::Positions, Coral::BufferType::VERTEX_BUFFER);
	auto [normalBuffer, normalBufferView]	  = ::createBuffer(context.get(), Cube::Normals, Coral::BufferType::VERTEX_BUFFER);
	auto [uvBuffer, uvBufferView]			  = ::createBuffer(context.get(), Cube::Texcoords, Coral::BufferType::VERTEX_BUFFER);
	auto [indexBuffer, indexBufferView]		  = ::createBuffer(context.get(), Cube::Indices, Coral::BufferType::INDEX_BUFFER);

	context->getTransferQueue()->waitIdle();
	std::array shaderModules = { vertexShader.get(), fragmentShader.get() };

	Coral::PipelineStateCreateConfig pipelineStateConfig{};
	pipelineStateConfig.shaderModules			= shaderModules;
	pipelineStateConfig.polygonMode				= Coral::PolygonMode::SOLID;
	pipelineStateConfig.topology				= Coral::Topology::TRIANGLE_LIST;
	pipelineStateConfig.faceCullingMode			= Coral::FaceCullingModes::BackFaceCulling;
	pipelineStateConfig.framebufferSignature	= swapchain->getFramebufferSignature();

	auto pipelineState = context->createPipelineState(pipelineStateConfig).value();

	auto renderFinishedSemaphore = context->createSemaphore().value();


	auto before = std::chrono::system_clock::now();

	auto rotation          = 0.f;
	auto rotationPerSecond = 90.f;

	// Update the displayed frame time every second
	float displayedFrameTimeUpdateInterval = 1.f;
	float timeSinceLastFrameTimeUpdate     = displayedFrameTimeUpdateInterval;
	float displayedFrameTime               = 0;

	// Start the game loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplCoral_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		auto info = swapchain->acquireNextSwapchainImage(nullptr);

		int width, height;
		glfwGetWindowSize(window, &width, &height);

		auto now      = std::chrono::system_clock::now();
		auto duration = now - before;
		before        = now;

		float deltaT = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / 1e+9f;

		timeSinceLastFrameTimeUpdate  += deltaT;
		if (timeSinceLastFrameTimeUpdate > displayedFrameTimeUpdateInterval)
		{
			timeSinceLastFrameTimeUpdate -= displayedFrameTimeUpdateInterval;
			displayedFrameTime = deltaT;
		}

		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10, 10), 0, ImVec2(1, 0));
		if (ImGui::Begin("Perf. Overlay", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize |  ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing))
		{
			if (ImGui::BeginTable("Perf. Overlay Table", 3, ImGuiTableFlags_SizingStretchProp))
			{
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::Text("FPS");

				ImGui::TableNextColumn();
				ImGui::Text(":");

				ImGui::TableNextColumn();
				ImGui::Text("%d", static_cast<int>(1.f / displayedFrameTime));

				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::Text("Frame time (ms)");

				ImGui::TableNextColumn();
				ImGui::Text(":");

				ImGui::TableNextColumn();
				ImGui::Text("%.2f", displayedFrameTime * 1000);

				ImGui::EndTable();
			}
		}

		ImGui::End();
		


		rotation += rotationPerSecond * deltaT;

		modelMatrix = glm::rotate(glm::mat4(1), glm::radians(rotation), glm::vec3(0, 1, 0));

		projectionMatrix = glm::perspective(fov, static_cast<float>(width) / height, nearPlane, farPlane);

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Render();

		uniformBlock.setMat44F("modelViewProjectionMatrix", projectionMatrix * viewMatrix * modelMatrix);
		uniformBlock.setMat33F("normalMatrix", glm::mat3(glm::transpose(glm::inverse(modelMatrix))));

		updateUniformBuffer(*uniformBuffer, uniformBlock);

		Coral::CommandBufferCreateConfig commandBufferConfig{};
		Coral::CommandBufferPtr commandBuffer(queue->createCommandBuffer(commandBufferConfig).value());
		commandBuffer->begin();

		Coral::BeginRenderPassInfo beginRenderPassInfo{};
		beginRenderPassInfo.framebuffer = info.framebuffer;
		Coral::ClearColor clearColor	= { Coral::ClearOp::CLEAR, { 1.f, 1.f, 1.f, 1.f }, 0 };
		Coral::ClearDepth clearDepth	= { Coral::ClearOp::CLEAR, 1.f, 0 };
		beginRenderPassInfo.clearColor	= { &clearColor, 1 };
		beginRenderPassInfo.clearDepth	= clearDepth;

		commandBuffer->cmdBeginRenderPass(beginRenderPassInfo);

		commandBuffer->cmdSetViewport({ 0, 0, static_cast<uint32_t>(width),  static_cast<uint32_t>(height),  0.f,  1.f , Coral::ViewportMode::Y_UP});

		commandBuffer->cmdBindPipeline(pipelineState.get());

		commandBuffer->cmdBindDescriptor(uniformBuffer.get(), 0);
		commandBuffer->cmdBindDescriptor(texture.get(), sampler.get(),  1);
		
		commandBuffer->cmdBindVertexBuffer(positionBufferView.get(), 0);
		commandBuffer->cmdBindVertexBuffer(normalBufferView.get(), 1);
		commandBuffer->cmdBindVertexBuffer(uvBufferView.get(), 2);
		commandBuffer->cmdBindIndexBuffer(indexBufferView.get());

		Coral::DrawIndexInfo drawInfo{};
		drawInfo.firstIndex = 0;
		drawInfo.indexCount = indexBufferView->count();
		commandBuffer->cmdDrawIndexed(drawInfo);

		ImGui_ImplCoral_RenderDrawData(ImGui::GetDrawData(), commandBuffer.get());

		commandBuffer->cmdEndRenderPass();

		commandBuffer->end();

		auto renderFinishedSemaphorePtr = renderFinishedSemaphore.get();

		Coral::CommandBufferSubmitInfo submitInfo{};
		auto cb                     = commandBuffer.get();
		submitInfo.commandBuffers   = { &cb, 1 };
		submitInfo.waitSemaphores   = { &info.imageAvailableSemaphore, 1 };
		submitInfo.signalSemaphores = { &renderFinishedSemaphorePtr, 1 };
		queue->submit(submitInfo, nullptr);

		Coral::PresentInfo presentInfo{};
		presentInfo.swapchain      = swapchain.get();
		presentInfo.waitSemaphores = { &renderFinishedSemaphorePtr, 1 };
		queue->submit(presentInfo);

		queue->waitIdle();
	}

	ImGui_ImplCoral_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return EXIT_SUCCESS;
}

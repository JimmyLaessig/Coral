#include "Cube.hpp"

#include "SimpleVertexShader.hpp"
//#include "DiffuseTextureFragmentShader.hpp"
#include "OpacityMaskFragmentShader.hpp"

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

#include <Coral/Coral.h>
#include <Coral/ImGui_Impl_Coral.h>

#include <Coral/Util/RAII.hpp>
#include <Coral/Util/UniformBlockBuilder.hpp>

#include <backends/imgui_impl_glfw.h>

#include <iostream>
#include <array>
#include <chrono>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>
#include <ranges>

namespace
{

struct ShaderSource
{
	std::string vertexShader;
	std::string fragmentShader;
};

void
printShaderSource(const std::string& s)
{
	auto lines = std::views::split(s, '\n');

	for (const auto& [i, line] : std::views::enumerate(lines))
	{
		auto lineNumber = std::to_string(i + 1);
		lineNumber.resize(3, ' ');
		std::cout << lineNumber << ": " << std::string_view(line.begin(), line.end()) << std::endl;
	}
}


inline std::optional<ShaderSource>
shaderSource()
{
	ShaderSource shaderSource;

	{
		csl::Examples::SimpleVertexShader vertexShader;
		csl::ShaderGraph shaderGraph(vertexShader);

		csl::CompilerSPV compiler;
		auto result = compiler.Compile(shaderGraph, csl::ShaderStage::VERTEX);

		std::cout << "-------------------- Vertex shader --------------------" << std::endl;
		printShaderSource(compiler.GetShaderSourceGLSL().shaderCode);
		
		if (!result)
		{
			std::cerr << result.error().message << std::endl;
			return {};
		}
		else
		{
			shaderSource.vertexShader = std::move(result->shaderCode);
		}
	}

	{
		csl::Examples::OpacityMaskFragmentShader fragmentShader;
		csl::ShaderGraph shaderGraph(fragmentShader);
		csl::CompilerSPV compiler;
		auto result = compiler.Compile(shaderGraph, csl::ShaderStage::FRAGMENT);

		std::cout << "-------------------- Fragment shader --------------------" << std::endl;
		printShaderSource(compiler.GetShaderSourceGLSL().shaderCode);

		if (!result)
		{
			std::cerr << result.error().message << std::endl;
			return {};
		}
		else
		{
			shaderSource.fragmentShader = std::move(result->shaderCode);
		}
	}

	return { std::move(shaderSource) };
}


struct Image
{
	std::vector<std::byte> data;

	uint32_t width{ 0 };
	uint32_t height{ 0 };

	CoPixelFormat format{ CO_PIXEL_FORMAT_RGBA8_UI };
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
			case 1:	image.format = CO_PIXEL_FORMAT_R8_UI;		break;
			case 2:	image.format = CO_PIXEL_FORMAT_RG8_UI;		break;
			case 3:	image.format = CO_PIXEL_FORMAT_RGB8_UI;		break;
			case 4:	image.format = CO_PIXEL_FORMAT_RGBA8_UI;	break;
			default:
				return false;
		}
		break;
	case 2:
		switch (c)
		{
			case 1:	image.format = CO_PIXEL_FORMAT_R16_UI;		break;
			case 2:	image.format = CO_PIXEL_FORMAT_RG16_UI;		break;
			case 3:	image.format = CO_PIXEL_FORMAT_RGB16_UI;	break;
			case 4:	image.format = CO_PIXEL_FORMAT_RGBA16_UI;	break;
			default:
				return false;
		}
		break;
	}

	return true;
}

template<typename T>
CoAttributeFormat toAttributeFormat();

template<> CoAttributeFormat toAttributeFormat<glm::vec3>() { return CO_ATTRIBUTE_FORMAT_VEC3F; }
template<> CoAttributeFormat toAttributeFormat<glm::vec2>() { return CO_ATTRIBUTE_FORMAT_VEC2F; }
template<> CoAttributeFormat toAttributeFormat<uint32_t>()  { return CO_ATTRIBUTE_FORMAT_UINT32; }
template<> CoAttributeFormat toAttributeFormat<uint16_t>()  { return CO_ATTRIBUTE_FORMAT_UINT16; }


template<typename T, size_t S>
std::shared_ptr<CoBuffer_T>
createBuffer(CoContext context, const std::array<T, S>& elements,CoBufferType type)
{
	CoBufferCreateConfig bufferConfig{};
	bufferConfig.size       = elements.size() * sizeof(T);
	bufferConfig.type       = type;
	bufferConfig.cpuVisible = false;
	
	Coral::BufferPtr buffer;
	if (coContextCreateBuffer(context, &bufferConfig, std::out_ptr(buffer)) != CO_SUCCESS)
	{
		return nullptr;
	}

	CoCommandQueue queue{ nullptr };
	if (coContextGetTransferQueue(context, &queue) != CO_SUCCESS)
	{
		return nullptr;
	}

	CoCommandBufferCreateConfig commandBufferConfig{};
	Coral::CommandBufferPtr commandBuffer;
	if (coCommandQueueCreateCommandBuffer(queue, &commandBufferConfig, std::out_ptr(commandBuffer)) != CO_SUCCESS)
	{
		return nullptr;
	}

	CoUpdateBufferDataInfo updateInfo{};
	updateInfo.buffer    = buffer.get();
	updateInfo.offset    = 0;
	updateInfo.pData     = reinterpret_cast<const CoByte*>(elements.data());
	updateInfo.dataCount = static_cast<uint32_t>(elements.size() * sizeof(T));

	coCommandBufferBegin(commandBuffer.get());
	coCommandBufferUpdateBufferData(commandBuffer.get(), &updateInfo);
	coCommandBufferEnd(commandBuffer.get());

	CoCommandBufferSubmitInfo submitInfo{};
	submitInfo.pCommandBuffers    = std::inout_ptr(commandBuffer);
	submitInfo.commandBufferCount = 1;

	Coral::FencePtr fence;
	CoFenceCreateConfig fenceConfig{};
	if (coContextCreateFence(context, &fenceConfig, std::out_ptr(fence)) != CO_SUCCESS)
	{
		return nullptr;
	}

coCommandQueueSubmit(queue, &submitInfo, fence.get());

coFenceWait(fence.get());

return buffer;
}


using UniformBlockBuilder = Coral::UniformBlockBuilder<glm::vec2, glm::vec3, glm::vec4, glm::ivec2, glm::ivec3, glm::ivec4, glm::mat3, glm::mat4>;

void
updateUniformBuffer(CoBuffer buffer, const UniformBlockBuilder& block)
{
	CoByte* mapped{ nullptr };
	coBufferMap(buffer, &mapped);

	auto data = block.data();
	std::memcpy(mapped, data.data(), data.size());

	coBufferUnMap(buffer);
}


Coral::BufferPtr
createUniformBuffer(CoContext context, const UniformBlockBuilder& block)
{
	CoBufferCreateConfig config{};
	config.cpuVisible = true;
	config.size       = block.data().size();
	config.type       = CO_BUFFER_TYPE_UNIFORM;

	Coral::BufferPtr buffer;
	if (coContextCreateBuffer(context, &config, std::out_ptr(buffer)) != CO_SUCCESS)
	{
		return nullptr;
	}

	updateUniformBuffer(buffer.get(), block);

	return buffer;
}


std::pair<Coral::ImagePtr, Coral::SamplerPtr>
createTexture(CoContext context, const std::string& path)
{
	Image img;
	if (!loadImage(path.c_str(), img))
	{
		return {};
	}

	CoImageCreateConfig imageConfig{};
	imageConfig.extent.width  = img.width;
	imageConfig.extent.height = img.height;
	imageConfig.format        = img.format;
	imageConfig.hasMipMaps    = true;
	imageConfig.usageHint     = CO_IMAGE_USAGE_HINT_SHADER_READ_ONLY;

	Coral::ImagePtr image;
	if (coContextCreateImage(context, &imageConfig, std::out_ptr(image)) != CO_SUCCESS)
	{
		return { nullptr, nullptr };
	}

	CoCommandQueue queue;
	if (coContextGetTransferQueue(context, &queue) != CO_SUCCESS)
	{
		return { nullptr, nullptr };
	}

	CoCommandBufferCreateConfig commandBufferConfig{};
	Coral::CommandBufferPtr commandBuffer;
	if (coCommandQueueCreateCommandBuffer(queue, &commandBufferConfig, std::out_ptr(commandBuffer)) != CO_SUCCESS)
	{
		return { nullptr, nullptr };
	}

	CoUpdateImageDataInfo updateInfo{};
	updateInfo.image     = image.get();
	updateInfo.pData     = reinterpret_cast<const CoByte*>(img.data.data());
	updateInfo.dataCount = static_cast<uint32_t>(img.data.size());

	coCommandBufferBegin(commandBuffer.get());
	coCommandBufferUpdateImageData(commandBuffer.get(), &updateInfo);

	if (coImageGetMipLevelCount(image.get()) > 1)
	{
		coCommandBufferGenerateMipMaps(commandBuffer.get(), image.get());
	}

	coCommandBufferEnd(commandBuffer.get());


	CoCommandBufferSubmitInfo submitInfo{};
	submitInfo.pCommandBuffers = std::inout_ptr(commandBuffer);
	submitInfo.commandBufferCount = 1;

	CoFenceCreateConfig fenceConfig{};
	Coral::FencePtr fence;
	if (coContextCreateFence(context, &fenceConfig, std::out_ptr(fence)) != CO_SUCCESS)
	{
		return { nullptr, nullptr };
	}

	coCommandQueueSubmit(queue, &submitInfo, fence.get());
	coFenceWait(fence.get());
	
	CoSamplerCreateConfig samplerConfig{};
	samplerConfig.wrapMode     = CO_WRAP_MODE_CLAMP_TO_EDGE;
	samplerConfig.minFilter    = CO_FILTER_LINEAR;
	samplerConfig.magFilter    = CO_FILTER_LINEAR;
	samplerConfig.mipmapFilter = CO_FILTER_LINEAR;

	Coral::SamplerPtr sampler;
	if (coContextCreateSampler(context, &samplerConfig, std::out_ptr(sampler)) != CO_SUCCESS)
	{
		return { nullptr, nullptr };
	}
	
	return { std::move(image), std::move(sampler) };
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

	CoContextCreateConfig contextConfig{};
	contextConfig.graphicsAPI      = CO_GRAPHICS_API_VULKAN;
	contextConfig.pApplicationName = "Coral Demo Application";

	Coral::ContextPtr context;
	if (coCreateContext(&contextConfig, std::out_ptr(context)) != CO_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	auto depthFormat = CO_PIXEL_FORMAT_DEPTH24_STENCIL8;

	CoSwapchainCreateConfig swapchainConfig{};
	swapchainConfig.nativeWindowHandle  = glfwGetWin32Window(window);
	swapchainConfig.depthFormat         = &depthFormat;
	swapchainConfig.format              = CO_PIXEL_FORMAT_RGBA8_SRGB;
	swapchainConfig.minImageCount       = 2;
	
	Coral::SwapchainPtr swapchain;
	if (coContextCreateSwapchain(context.get(), &swapchainConfig, std::out_ptr(swapchain)) != CO_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	CoFenceCreateConfig fenceConfig{};
	Coral::FencePtr fence;
	if (coContextCreateFence(context.get(), &fenceConfig, std::out_ptr(fence)) != CO_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	CoCommandQueue queue{ nullptr };
	if (coContextGetGraphicsQueue(context.get(), &queue) != CO_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	CoSemaphoreCreateConfig semaphoreConfig{};
	Coral::SemaphorePtr renderFinishedSemaphore;
	if (coContextCreateSemaphore(context.get(), &semaphoreConfig, std::out_ptr(renderFinishedSemaphore)) != CO_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	auto shader = shaderSource();
	if (!shader)
	{
		return EXIT_FAILURE;
	}

	CoShaderModuleCreateConfig vertexShaderConfig{};
	vertexShaderConfig.pName       = "VertexShader";
	vertexShaderConfig.stage       = CO_SHADER_STAGE_VERTEX;
	vertexShaderConfig.pEntryPoint = "main";
	vertexShaderConfig.pSource     = reinterpret_cast<const CoByte*>(shader->vertexShader.c_str());
	vertexShaderConfig.sourceCount = static_cast<uint32_t>(shader->vertexShader.size());

	Coral::ShaderModulePtr vertexShader;
	if (coContextCreateShaderModule(context.get(), &vertexShaderConfig, std::out_ptr(vertexShader)) != CO_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	CoShaderModuleCreateConfig fragmentShaderConfig{};
	fragmentShaderConfig.pName       = "FragmentShader";
	fragmentShaderConfig.stage       = CO_SHADER_STAGE_FRAGMENT;
	fragmentShaderConfig.pEntryPoint = "main";
	fragmentShaderConfig.pSource     = reinterpret_cast<const CoByte*>(shader->fragmentShader.c_str());
	fragmentShaderConfig.sourceCount = static_cast<uint32_t>(shader->fragmentShader.size());
	
	Coral::ShaderModulePtr fragmentShader;
	if (coContextCreateShaderModule(context.get(), &fragmentShaderConfig, std::out_ptr(fragmentShader)) != CO_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

	ImGui_ImplGlfw_InitForVulkan(window, true);

	ImGui_ImplCoral_InitInfo initInfo{};
	initInfo.context             = context.get();
	initInfo.swapchainImageCount = coSwapchainGetImageCount(swapchain.get());
	coSwapchainGetFramebufferLayout(swapchain.get(), &initInfo.framebufferSignature);

	if (ImGui_ImplCoral_Init(&initInfo) != CO_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	if (ImGui_ImplCoral_CreateFontsTexture(context.get()) != CO_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	CoShaderModuleLayout layout{};
	coShaderModuleGetLayout(vertexShader.get(), &layout);

	std::span attributes(layout.pInputAttributeBindingInfos, layout.inputAttributeBindingInfoCount);

	auto positionAttribute  = std::ranges::find_if(attributes, [](const CoAttributeBindingInfo& info) { return info.location == 0; })->location;
	auto normalsAttribute   = std::ranges::find_if(attributes, [](const CoAttributeBindingInfo& info) { return info.location == 1; })->location;
	auto texcoordsAttribute = std::ranges::find_if(attributes, [](const CoAttributeBindingInfo& info) { return info.location == 2; })->location;


	glm::mat4 modelMatrix(1.f);

	auto viewMatrix       = glm::lookAt(glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	auto fov			  = glm::radians(65.f);
	auto nearPlane		  = 0.01f;
	auto farPlane		  = 1000.f;
	auto projectionMatrix = glm::perspective(fov, static_cast<float>(WIDTH) / HEIGHT, nearPlane, farPlane);

	UniformBlockBuilder matrices(layout.pDescriptorBindingInfos[0].block);
	matrices.set("modelViewProjectionMatrix", projectionMatrix * viewMatrix * modelMatrix);
	matrices.set("normalMatrix", glm::transpose(glm::inverse(glm::mat3(modelMatrix))));
	auto matricesBuffer = createUniformBuffer(context.get(), matrices);
	
	coShaderModuleGetLayout(fragmentShader.get(), &layout);
	UniformBlockBuilder lightData(layout.pDescriptorBindingInfos[0].block);
	lightData.set("lightColor", glm::vec4{ 1.f, 1.f, 1.f, 1.f });
	lightData.set("lightDirection", glm::normalize(glm::vec4{ 1.f, 1.f, 1.f, 1.f }));
	auto lightDataBuffer = createUniformBuffer(context.get(), lightData);

	auto [texture, sampler] = createTexture(context.get(), "resources/uvtest.png");

	auto positions   = ::createBuffer(context.get(), Cube::Positions, CO_BUFFER_TYPE_VERTEX);
	auto normals	 = ::createBuffer(context.get(), Cube::Normals,   CO_BUFFER_TYPE_VERTEX);
	auto texcoords 	 = ::createBuffer(context.get(), Cube::Texcoords, CO_BUFFER_TYPE_VERTEX);
	auto indices	 = ::createBuffer(context.get(), Cube::Indices,   CO_BUFFER_TYPE_INDEX);
	auto indexFormat = CO_INDEX_FORMAT_UINT16;

	CoPipelineStateCreateConfig pipelineStateConfig{};
	pipelineStateConfig.vertexShaderModule	 = vertexShader.get();
	pipelineStateConfig.fragmentShaderModule = fragmentShader.get();

	pipelineStateConfig.polygonMode				    = CO_POLYGON_MODE_SOLID;
	pipelineStateConfig.topology				    = CO_TOPOLOGY_TRIANGLE_LIST;
	pipelineStateConfig.faceCullingMode.cullMode    = CO_CULL_MODE_BACK;
	pipelineStateConfig.faceCullingMode.orientation = CO_FRONT_FACE_ORIENTATION_CCW;	

	pipelineStateConfig.depthTestMode.writeDepth           = true;
	pipelineStateConfig.depthTestMode.compareOp            = CO_COMPARE_OP_LESS;
	pipelineStateConfig.depthTestMode.polygonOffset.factor = 0.f;
	pipelineStateConfig.depthTestMode.polygonOffset.units  = 0.f;

	pipelineStateConfig.blendMode.blendOp    = CO_BLEND_OP_ADD;
	pipelineStateConfig.blendMode.srcFactor  = CO_BLEND_FACTOR_ONE;
	pipelineStateConfig.blendMode.destFactor = CO_BLEND_FACTOR_ZERO;
	coSwapchainGetFramebufferLayout(swapchain.get(), &pipelineStateConfig.framebufferLayout);

	Coral::PipelineStatePtr pipelineState;
	if (coContextCreatePipelineState(context.get(), &pipelineStateConfig, std::out_ptr(pipelineState)) != CO_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	auto before = std::chrono::system_clock::now();

	auto rotation          = 0.f;
	auto rotationPerSecond = 0.25f;

	// Update the displayed frame time every second
	float displayedFrameTimeUpdateInterval = 1.f;
	float timeSinceLastFrameTimeUpdate     = displayedFrameTimeUpdateInterval;
	float displayedFrameTime               = 0;

	// Start the game loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		CoSwapchainImageInfo info{};
		if (coSwapchainAcquireNextImage(swapchain.get(), nullptr, &info) != CO_SUCCESS)
		{
			return EXIT_FAILURE;
		}

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
				ImGui::Text("%d", static_cast<int>(1.f / displayedFrameTime));

				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::Text("Frame time (ms)");

				ImGui::TableNextColumn();
				ImGui::Text(":");

				ImGui::TableNextColumn();
				ImGui::Text("%.2f", displayedFrameTime * 1000);

				ImGui::EndTable();
			
				ImGui::Text("Rotation speed");
				ImGui::PushID("rotationPerSecond");
				ImGui::SliderFloat("", &rotationPerSecond, -1.f, 1.f);
				ImGui::PopID();
			}
		}

		ImGui::End();

		rotation += (rotationPerSecond * 360.f) * deltaT;

		modelMatrix = glm::rotate(glm::mat4(1), glm::radians(rotation), glm::vec3(0, 1, 0));

		projectionMatrix = glm::perspective(fov, static_cast<float>(width) / height, nearPlane, farPlane);

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Render();

		matrices.set("modelViewProjectionMatrix", projectionMatrix * viewMatrix * modelMatrix);
		matrices.set("normalMatrix", glm::mat3(glm::transpose(glm::inverse(modelMatrix))));
		updateUniformBuffer(matricesBuffer.get(), matrices);

		CoCommandBufferCreateConfig commandBufferConfig{};
	
		Coral::CommandBufferPtr commandBuffer;
		if (coCommandQueueCreateCommandBuffer(queue, &commandBufferConfig, std::out_ptr(commandBuffer)) != CO_SUCCESS)
		{
			return EXIT_FAILURE;
		}

		coCommandBufferBegin(commandBuffer.get());

		CoClearColor clearColor{ 0, CO_CLEAR_OP_CLEAR, { 1.f, 1.f, 1.f, 1.f } };
		CoClearDepthStencil clearDepth{ CO_CLEAR_OP_CLEAR, 1.f, 0 };

		CoBeginRenderPassInfo beginRenderPassInfo{};
		beginRenderPassInfo.framebuffer       = info.framebuffer;
		beginRenderPassInfo.pClearColors	  = &clearColor;
		beginRenderPassInfo.clearColorsCount  = 1;
		beginRenderPassInfo.clearDepthStencil = &clearDepth;

		coCommandBufferBeginRenderPass(commandBuffer.get(), &beginRenderPassInfo);
	
		CoViewportInfo viewport
		{
			CoRectangle{ 0, 0, static_cast<uint32_t>(width),  static_cast<uint32_t>(height) },
			0.f,  1.f
		};

		coCommandBufferSetViewport(commandBuffer.get(), &viewport);
		coCommandBufferBindPipeline(commandBuffer.get(), pipelineState.get());
		coCommandBufferBindUniformBuffer(commandBuffer.get(), matricesBuffer.get(), 0);
		coCommandBufferBindUniformBuffer(commandBuffer.get(), lightDataBuffer.get(), 1);
		coCommandBufferBindImage(commandBuffer.get(), texture.get(),  2);
		coCommandBufferBindSampler(commandBuffer.get(), sampler.get(), 2);

		coCommandBufferBindVertexBuffer(commandBuffer.get(), positions.get(), positionAttribute,  0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
		coCommandBufferBindVertexBuffer(commandBuffer.get(), normals.get(),   normalsAttribute,   0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC3F));
		coCommandBufferBindVertexBuffer(commandBuffer.get(), texcoords.get(), texcoordsAttribute, 0, coAttributeFormatGetSizeInBytes(CO_ATTRIBUTE_FORMAT_VEC2F));

		coCommandBufferBindIndexBuffer(commandBuffer.get(), indices.get(), indexFormat, 0);

		coCommandBufferDrawIndexed(commandBuffer.get(), 0, static_cast<uint32_t>(Cube::Indices.size()));

		ImGui_ImplCoral_NewFrame(context.get());

		ImGui_ImplCoral_RenderDrawData(context.get(), ImGui::GetDrawData(), commandBuffer.get());

		coCommandBufferEndRenderPass(commandBuffer.get());
		coCommandBufferEnd(commandBuffer.get());

		auto renderFinishedSemaphorePtr = renderFinishedSemaphore.get();

		CoCommandBufferSubmitInfo submitInfo{};
		auto cb                       = commandBuffer.get();
		submitInfo.pCommandBuffers    = &cb;
		submitInfo.commandBufferCount = 1;
		submitInfo.pWaitSemaphores    = &info.imageAcquiredSemaphore;
		submitInfo.waitSemaphoreCount = 0;

		submitInfo.pSignalSemaphores    = &renderFinishedSemaphorePtr;
		submitInfo.signalSemaphoreCount = 0;

		if (coCommandQueueSubmit(queue, &submitInfo, nullptr) != CO_SUCCESS)
		{
			return EXIT_FAILURE;
		}

		coCommandQueueWaitIdle(queue);

		CoPresentInfo presentInfo{};
		presentInfo.swapchain          = swapchain.get();
		presentInfo.pWaitSemaphores    = &info.imageAcquiredSemaphore;
		presentInfo.waitSemaphoreCount = 1;
		
		if (coCommandQueuePresent(queue, &presentInfo) != CO_SUCCESS)
		{
			return EXIT_FAILURE;
		}

		if (coCommandQueueWaitIdle(queue) != CO_SUCCESS)
		{
			return EXIT_FAILURE;
		}
	}

	ImGui_ImplCoral_Shutdown(context.get());
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	return EXIT_SUCCESS;
}
#include "VulkanDevice.hpp"
#include "../Utils/Exceptions.hpp"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <array>
#include <fstream>

#define CMP "VulkanDevice"

using namespace Engine;

bool VulkanDevice::getValidationLayerSupported() {
    // Enumerate all available instance layers.
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layerProperties(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

    for (auto prop : layerProperties) {
        if (std::string(prop.layerName) == "VK_LAYER_LUNARG_standard_validation") {
            return true;
        }
    }

    return false;
}

void VulkanDevice::initInstance(const std::string& name, std::vector<const char*> instanceLayers,
                                std::vector<const char*> instanceExtensions) {
    VezApplicationInfo appInfo = {nullptr, name.c_str(), VK_MAKE_VERSION(1, 0, 0), "", VK_MAKE_VERSION(0, 0, 0)};
    VezInstanceCreateInfo createInfo = {nullptr,
                                        &appInfo,
                                        static_cast<uint32_t>(instanceLayers.size()),
                                        instanceLayers.data(),
                                        static_cast<uint32_t>(instanceExtensions.size()),
                                        instanceExtensions.data()};

    auto result = vezCreateInstance(&createInfo, &instance);
    if (result != VK_SUCCESS) {
        EXCEPTION("vkCreateInstance failed result: {}", result);
    }

    // Enumerate all attached physical devices.
    uint32_t physicalDeviceCount = 0;
    vezEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0) {
        EXCEPTION("No Vulkan physical devices found");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vezEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

    for (auto pd : physicalDevices) {
        VkPhysicalDeviceProperties properties = {};
        vezGetPhysicalDeviceProperties(pd, &properties);
        Log::i(CMP, "Physical device name: {}", properties.deviceName);
    }

    // Select the physical device.
    physicalDevice = physicalDevices[physicalDeviceIndex];

    // Get the physical device information.
    VkPhysicalDeviceProperties properties = {};
    vezGetPhysicalDeviceProperties(physicalDevice, &properties);
    Log::i(CMP, "Device name: {}", properties.deviceName);
}

void VulkanDevice::initSurface(GLFWwindow* window) {
    // Create a surface from the GLFW window handle.
    auto result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (result != VK_SUCCESS) {
        EXCEPTION("Failed to create Vulkan surface (glfwCreateWindowSurface)");
    }

    // Create the Vulkan device handle.
    std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    for (auto i = 0U; i < deviceExtensions.size(); ++i) {
        extensions.push_back(deviceExtensions[i].c_str());
    }

    VezDeviceCreateInfo deviceCreateInfo = {nullptr, 0, nullptr, static_cast<uint32_t>(extensions.size()),
                                            extensions.data()};
    result = vezCreateDevice(physicalDevice, &deviceCreateInfo, instance, &device);
    if (result != VK_SUCCESS) {
        EXCEPTION("Failed to create Vulkan device (vezCreateDevice)");
    }

    // Create the swapchain.
    VezSwapchainCreateInfo swapchainCreateInfo = {};
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    swapchainCreateInfo.tripleBuffer = VK_TRUE;
    result = vezCreateSwapchain(device, &swapchainCreateInfo, &swapchain);
    if (result != VK_SUCCESS) {
        EXCEPTION("Failed to create Vulkan swapchain (vezCreateSwapchain)");
    }

    vezDeviceWaitIdle(device);
    vezSwapchainSetVSync(swapchain, true);
}

void VulkanDevice::resizeDefaultFramebuffer(const Vector2i& size) {
    vkDeviceWaitIdle(device);
}

VulkanTexture::Format VulkanDevice::getSwapchainFormat() const {
    VkSurfaceFormatKHR swapchainFormat = {};
    vezGetSwapchainSurfaceFormat(swapchain, &swapchainFormat);

    return swapchainFormat.format;
}

VulkanFramebuffer VulkanDevice::createFramebuffer(const Vector2i& size,
                                                  const std::vector<VulkanFramebufferAttachment>& attachments) {
    // Create the m_framebuffer.
    std::vector<VkImageView> imageViews;
    for (auto& attachment : attachments) {
        imageViews.push_back(attachment.texture.get().getView());
    }

    VezFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
    framebufferCreateInfo.pAttachments = imageViews.data();
    framebufferCreateInfo.width = size.x;
    framebufferCreateInfo.height = size.y;
    framebufferCreateInfo.layers = 1;

    VezFramebuffer framebufferHandle;
    const auto result = vezCreateFramebuffer(device, &framebufferCreateInfo, &framebufferHandle);
    if (result != VK_SUCCESS) {
        EXCEPTION("vezCreateFramebuffer failed with result: {}", result);
    }

    return VulkanFramebuffer(device, framebufferHandle);
}

void VulkanDevice::deviceWaitIdle() {
    // Wait for all device operations to complete.
    vezDeviceWaitIdle(device);
}

void VulkanDevice::reset() {
    // Destroy the swapchain.
    vezDestroySwapchain(device, swapchain);

    // Destroy device.
    vezDestroyDevice(device);

    // Destroy surface.
    vkDestroySurfaceKHR(instance, surface, nullptr);

    // Destroy instance.
    vezDestroyInstance(instance);

    if (commandBuffer != VK_NULL_HANDLE) {
        vezFreeCommandBuffers(device, 1, &commandBuffer);
    }
}

void VulkanDevice::setViewport(const Vector2i& pos, const Vector2i& size) {
    VkViewport view = {
        static_cast<float>(pos.x),
        static_cast<float>(pos.y),
        static_cast<float>(size.x),
        static_cast<float>(size.y),
        0.0f,
        1.0f,
    };
    vezCmdSetViewport(0, 1, &view);
}

void VulkanDevice::setScissor(const Vector2i& pos, const Vector2i& size) {
    VkRect2D scissor = {{static_cast<int32_t>(pos.x), static_cast<int32_t>(pos.y)},
                        {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)}};
    vezCmdSetScissor(0, 1, &scissor);
}

void VulkanDevice::setViewportState() {
    vezCmdSetViewportState(1);
}

void VulkanDevice::beginRenderPass(VulkanFramebuffer& fbo,
                                   const std::vector<VulkanFramebufferAttachmentReference>& attachments) {
    std::vector<VezAttachmentReference> attachmentReferences = {};
    attachmentReferences.resize(attachments.size());

    for (size_t i = 0; i < attachments.size(); i++) {
        attachmentReferences[i].clearValue = attachments[i].clearValue;
        attachmentReferences[i].loadOp = attachments[i].loadOp;
        attachmentReferences[i].storeOp = attachments[i].storeOp;
    }

    // Begin a render pass.
    VezRenderPassBeginInfo beginInfo = {};
    beginInfo.framebuffer = fbo.getHandle();
    beginInfo.attachmentCount = static_cast<uint32_t>(attachmentReferences.size());
    beginInfo.pAttachments = attachmentReferences.data();
    vezCmdBeginRenderPass(&beginInfo);
}

void VulkanDevice::nextSubpass() {
    vezCmdNextSubpass();
}

void VulkanDevice::endRenderPass() {
    vezCmdEndRenderPass();
}

void VulkanDevice::setRasterization(VkPolygonMode mode, VkCullModeFlags cullMode, VkFrontFace frontFace) {
    VezRasterizationState rasterizationState = {};
    rasterizationState.polygonMode = mode;
    rasterizationState.cullMode = cullMode;
    rasterizationState.frontFace = frontFace;
    vezCmdSetRasterizationState(&rasterizationState);
}

void VulkanDevice::setInputAssembly(VkPrimitiveTopology topology, bool restart) {
    VezInputAssemblyState inputAssemblyState = {};
    inputAssemblyState.topology = topology;
    inputAssemblyState.primitiveRestartEnable = restart ? VK_TRUE : VK_FALSE;
    vezCmdSetInputAssemblyState(&inputAssemblyState);
}

void VulkanDevice::setDepthStencilState(bool write, bool test, VkCompareOp compareOp) {
    VezPipelineDepthStencilState depthStencilState = {};
    depthStencilState.depthTestEnable = test;
    depthStencilState.depthWriteEnable = write;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    vezCmdSetDepthStencilState(&depthStencilState);
}

void VulkanDevice::setBlendState(const std::vector<VulkanBlendState>& blendStates) {
    VezColorBlendState colorBlendState = {};
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_SET;
    colorBlendState.attachmentCount = static_cast<uint32_t>(blendStates.size());
    colorBlendState.pAttachments = blendStates.data();

    vezCmdSetColorBlendState(&colorBlendState);
}

void VulkanDevice::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                               uint32_t firstInstance) {
    vezCmdDrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanDevice::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    vezCmdDraw(vertexCount, instanceCount, firstVertex, firstInstance);
}

VulkanBuffer VulkanDevice::createBuffer(VulkanBuffer::Type type, VulkanBuffer::Usage usage, size_t size) {
    return VulkanBuffer(device, type, usage, size);
}

VulkanTexture VulkanDevice::createTexture(const VulkanTexture::Descriptor& desc) {
    return VulkanTexture(device, desc);
}

VulkanVertexInputFormat
VulkanDevice::createVertexInputFormat(const std::vector<VulkanVertexInputFormat::Binding>& bindings) {
    return VulkanVertexInputFormat(device, bindings);
}

VkShaderModule VulkanDevice::CreateShaderModule(const std::string& code, const std::string& entryPoint,
                                                VkShaderStageFlagBits stage) {
    // Create the shader module.
    VezShaderModuleCreateInfo createInfo = {};
    createInfo.stage = stage;
    createInfo.codeSize = static_cast<uint32_t>(code.size());
    createInfo.pGLSLSource = code.c_str();
    createInfo.pEntryPoint = entryPoint.c_str();

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    auto result = vezCreateShaderModule(device, &createInfo, &shaderModule);
    if (result != VK_SUCCESS && shaderModule != VK_NULL_HANDLE) {
        // If shader module creation failed but error is from GLSL compilation, get the error log.
        uint32_t infoLogSize = 0;
        vezGetShaderModuleInfoLog(shaderModule, &infoLogSize, nullptr);

        std::string infoLog(infoLogSize, '\0');
        vezGetShaderModuleInfoLog(shaderModule, &infoLogSize, &infoLog[0]);

        vezDestroyShaderModule(device, shaderModule);

        EXCEPTION("Failed to compile shader error: {}", infoLog);
    }

    return shaderModule;
}

VulkanPipeline VulkanDevice::createPipeline(const std::vector<ShaderSource>& sources) {
    VezPipeline pipeline = VK_NULL_HANDLE;
    std::vector<VkShaderModule> shaderModules;

    // Create shader modules.
    std::vector<VezPipelineShaderStageCreateInfo> shaderStageCreateInfo(sources.size());
    for (auto i = 0U; i < sources.size(); ++i) {
        std::string glsl;

        if (!sources[i].path.empty()) {
            Log::d(CMP, "Loading shader: '{}'", sources[i].path);
            std::ifstream file(sources[i].path);
            if (!file) {
                EXCEPTION("Failed to open file: '{}'", sources[i].path);
            }

            glsl = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        } else {
            glsl = sources[i].glsl;
        }

        auto stage = static_cast<VkShaderStageFlagBits>(sources[i].type);

        auto shaderModule = CreateShaderModule(glsl, "main", stage);

        shaderStageCreateInfo[i].module = shaderModule;
        shaderStageCreateInfo[i].pEntryPoint = "main";
        shaderStageCreateInfo[i].pSpecializationInfo = nullptr;

        shaderModules.push_back(shaderModule);
    }

    // Determine if this is a compute only pipeline.
    bool isComputePipeline =
        (sources.size() == 1 && static_cast<VkShaderStageFlagBits>(sources[0].type) == VK_SHADER_STAGE_COMPUTE_BIT);

    // Create the graphics pipeline or compute pipeline.
    if (isComputePipeline) {
        VezComputePipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.pStage = shaderStageCreateInfo.data();
        if (vezCreateComputePipeline(device, &pipelineCreateInfo, &pipeline) != VK_SUCCESS) {
            EXCEPTION("vezCreateComputePipeline failed to create compute pipeline");
        }
    } else {
        VezGraphicsPipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStageCreateInfo.size());
        pipelineCreateInfo.pStages = shaderStageCreateInfo.data();
        if (vezCreateGraphicsPipeline(device, &pipelineCreateInfo, &pipeline) != VK_SUCCESS) {
            EXCEPTION("vezCreateGraphicsPipeline failed to create graphics pipeline");
        }
    }

    return VulkanPipeline(device, pipeline, std::move(shaderModules));
}

void VulkanDevice::bindVertexBuffer(const VulkanBuffer& buffer, const size_t offset) {
    if (!buffer) {
        EXCEPTION("Can not bind uninitialized vertex buffer");
    }
    VkDeviceSize off = offset;
    vezCmdBindVertexBuffers(0, 1, &buffer.getHandle(), &off);
}

void VulkanDevice::bindIndexBuffer(const VulkanBuffer& buffer, const size_t offset, const VkIndexType indexType) {
    if (!buffer) {
        EXCEPTION("Can not bind uninitialized index buffer");
    }
    vezCmdBindIndexBuffer(buffer.getHandle(), offset, indexType);
}

void VulkanDevice::bindUniformBuffer(const VulkanBuffer& buffer, const uint32_t binding, const size_t offset) {
    if (!buffer) {
        EXCEPTION("Can not bind uninitialized uniform buffer slot {}", binding);
    }
    vezCmdBindBuffer(buffer.getHandle(), 0, VK_WHOLE_SIZE, 0, binding, 0);
}

void VulkanDevice::bindPipeline(const VulkanPipeline& pipeline) {
    if (!pipeline) {
        EXCEPTION("Can not bind uninitialized pipeline");
    }
    vezCmdBindPipeline(pipeline.getHandle());
}

void VulkanDevice::bindVertexInputFormat(const VulkanVertexInputFormat& vertexInputFormat) {
    if (!vertexInputFormat) {
        EXCEPTION("Can not bind uninitialized vertex input format");
    }
    vezCmdSetVertexInputFormat(vertexInputFormat.getHandle());
}

void VulkanDevice::bindTexture(const VulkanTexture& texture, uint32_t binding) {
    if (!texture) {
        EXCEPTION("Can not bind uninitialized texture to slot {}", binding);
    }
    vezCmdBindImageView(texture.getView(), texture.getSampler(), 0, binding, 0);
}

void VulkanDevice::pushConstant(uint32_t offset, const Matrix4& value) {
    vezCmdPushConstants(offset, sizeof(Matrix4), &value);
}

void VulkanDevice::pushConstant(uint32_t offset, const Color4& value) {
    vezCmdPushConstants(offset, sizeof(Color4), &value);
}

void VulkanDevice::startCommandBuffer() {
    if (commandBuffer != VK_NULL_HANDLE) {
        vezFreeCommandBuffers(device, 1, &commandBuffer);
    }

    // Get the graphics queue handle.
    vezGetDeviceGraphicsQueue(device, 0, &graphicsQueue);

    // Create a command buffer handle.
    VezCommandBufferAllocateInfo allocInfo = {};
    allocInfo.queue = graphicsQueue;
    allocInfo.commandBufferCount = 1;
    if (vezAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        EXCEPTION("vezAllocateCommandBuffers failed");
    }

    // Begin command buffer recording.
    if (vezBeginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) != VK_SUCCESS) {
        EXCEPTION("vezBeginCommandBuffer failed");
    }
}

void VulkanDevice::endCommandBuffer() {
    // End command buffer recording.
    if (vezEndCommandBuffer() != VK_SUCCESS) {
        EXCEPTION("vezEndCommandBuffer failed");
    }
}

void VulkanDevice::submitQueue() {
    // Submit the command buffer to the graphics queue.
    VezSubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Request a wait semaphore to pass to present so it waits for rendering to complete.
    VkSemaphore semaphore = VK_NULL_HANDLE;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphore;
    if (vezQueueSubmit(graphicsQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
        EXCEPTION("vezQueueSubmit failed");
    }

    vezDeviceWaitIdle(device);
}

void VulkanDevice::submitPresentQueue(VulkanTexture& front) {
    // Submit the command buffer to the graphics queue.
    VezSubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Request a wait semaphore to pass to present so it waits for rendering to complete.
    VkSemaphore semaphore = VK_NULL_HANDLE;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphore;
    if (vezQueueSubmit(graphicsQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
        EXCEPTION("vezQueueSubmit failed");
    }

    vezDeviceWaitIdle(device);

    // Present the swapchain framebuffer to the window.
    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VezPresentInfo presentInfo = {};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &semaphore;
    presentInfo.pWaitDstStageMask = &waitDstStageMask;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    // presentInfo.pImages = &framebuffer.colorImage;
    presentInfo.pImages = &front.getHandle();
    if (vezQueuePresent(graphicsQueue, &presentInfo) != VK_SUCCESS) {
        EXCEPTION("vezQueuePresentKHR failed");
    }
}

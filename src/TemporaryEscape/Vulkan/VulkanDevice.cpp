#include "VulkanDevice.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Engine;

void VulkanDevice::init(GLFWwindow* window) {
}

void VulkanDevice::reset() {
    if (commandBuffer != VK_NULL_HANDLE) {
        vezFreeCommandBuffers(getDevice(), 1, &commandBuffer);
    }
}

VulkanBuffer VulkanDevice::createBuffer(VulkanBuffer::Type type, VulkanBuffer::Usage usage, size_t size) {
    return VulkanBuffer(getDevice(), type, usage, size);
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
    auto result = vezCreateShaderModule(getDevice(), &createInfo, &shaderModule);
    if (result != VK_SUCCESS && shaderModule != VK_NULL_HANDLE) {
        // If shader module creation failed but error is from GLSL compilation, get the error log.
        uint32_t infoLogSize = 0;
        vezGetShaderModuleInfoLog(shaderModule, &infoLogSize, nullptr);

        std::string infoLog(infoLogSize, '\0');
        vezGetShaderModuleInfoLog(shaderModule, &infoLogSize, &infoLog[0]);

        vezDestroyShaderModule(getDevice(), shaderModule);

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
        auto code = sources[i].glsl;
        auto stage = static_cast<VkShaderStageFlagBits>(sources[i].type);

        auto shaderModule = CreateShaderModule(code, "main", stage);

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
        if (vezCreateComputePipeline(getDevice(), &pipelineCreateInfo, &pipeline) != VK_SUCCESS) {
            EXCEPTION("vezCreateComputePipeline failed to create compute pipeline");
        }
    } else {
        VezGraphicsPipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStageCreateInfo.size());
        pipelineCreateInfo.pStages = shaderStageCreateInfo.data();
        if (vezCreateGraphicsPipeline(getDevice(), &pipelineCreateInfo, &pipeline) != VK_SUCCESS) {
            EXCEPTION("vezCreateGraphicsPipeline failed to create graphics pipeline");
        }
    }

    return VulkanPipeline(getDevice(), pipeline, std::move(shaderModules));
}

void VulkanDevice::bindVertexBuffer(VulkanBuffer& buffer, size_t offset) {
    VkDeviceSize off = offset;
    vezCmdBindVertexBuffers(0, 1, &buffer.getHandle(), &off);
}

void VulkanDevice::bindIndexBuffer(VulkanBuffer& buffer, size_t offset) {
    vezCmdBindIndexBuffer(buffer.getHandle(), offset, VK_INDEX_TYPE_UINT32);
}

void VulkanDevice::bindUniformBuffer(VulkanBuffer& buffer, size_t offset) {
    vezCmdBindBuffer(buffer.getHandle(), 0, VK_WHOLE_SIZE, 0, 0, 0);
}

void VulkanDevice::bindPipeline(VulkanPipeline& pipeline) {
    vezCmdBindPipeline(pipeline.getHandle());
}

void VulkanDevice::startCommandBuffer() {
    if (commandBuffer != VK_NULL_HANDLE) {
        vezFreeCommandBuffers(getDevice(), 1, &commandBuffer);
    }

    // Get the graphics queue handle.
    vezGetDeviceGraphicsQueue(getDevice(), 0, &graphicsQueue);

    // Create a command buffer handle.
    VezCommandBufferAllocateInfo allocInfo = {};
    allocInfo.queue = graphicsQueue;
    allocInfo.commandBufferCount = 1;
    if (vezAllocateCommandBuffers(getDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
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

    vezDeviceWaitIdle(getDevice());

    // Present the swapchain framebuffer to the window.
    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    auto swapchain = getSwapchain();
    auto srcImage = getColorAttachment();

    VezPresentInfo presentInfo = {};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &semaphore;
    presentInfo.pWaitDstStageMask = &waitDstStageMask;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImages = &srcImage;
    if (vezQueuePresent(graphicsQueue, &presentInfo) != VK_SUCCESS) {
        EXCEPTION("vezQueuePresentKHR failed");
    }
}

#include "Renderer.hpp"

#include <vulkan/vulkan_core.h>

#include <glm/mat4x4.hpp>
#include <list>
#include <memory>

#include "BufferManager.hpp"
#include "Context.hpp"
#include "ForwardPass.hpp"
#include "GeometryRenderer.hpp"
#include "Skybox.hpp"
#include "Swapchain.hpp"

using namespace render;

Renderer::Renderer() {
    try {
        forwardPass = std::make_unique<ForwardPass>();

        createCommandPool();
        createCommandBuffer();
        createSyncObjects();
    } catch (...) {
        cleanup();
        throw;
    }
}

void Renderer::cleanup() {
    if (imageAvailableSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(Context::get().getDevice(), imageAvailableSemaphore,
                           nullptr);
        imageAvailableSemaphore = VK_NULL_HANDLE;
    }

    if (renderFinishedSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(Context::get().getDevice(), renderFinishedSemaphore,
                           nullptr);
        renderFinishedSemaphore = VK_NULL_HANDLE;
    }

    if (inFlightFence != VK_NULL_HANDLE) {
        vkDestroyFence(Context::get().getDevice(), inFlightFence, nullptr);
        inFlightFence = VK_NULL_HANDLE;
    }

    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(Context::get().getDevice(), commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }

    if (forwardPass) {
        forwardPass->cleanup();
        forwardPass.reset();
    }
}

void Renderer::render(const Camera& camera, const Skybox& skybox,
                      const GeometryRenderer::LightInfo& lights,
                      std::list<GeometryModel> models,
                      std::list<UiModel> uiModels, bool windowResized) {
    vkWaitForFences(Context::get().getDevice(), 1, &inFlightFence, VK_TRUE,
                    UINT64_MAX);

    // The GPU is idle, flush pending buffers
    BufferManager::get().flushDeferOperations();

    Swapchain::Frame frame =
        Swapchain::get().acquireFrame(imageAvailableSemaphore);

    vkResetFences(Context::get().getDevice(), 1, &inFlightFence);

    vkResetCommandBuffer(commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error{"failed to begin recording command buffer!"};

    forwardPass->record(commandBuffer, frame, camera, skybox, lights, models,
                        uiModels);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error{"failed to record command buffer!"};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags WAIT_STAGES[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = WAIT_STAGES;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

    if (vkQueueSubmit(Context::get().getGraphicsQueue(), 1, &submitInfo,
                      inFlightFence) != VK_SUCCESS)
        throw std::runtime_error{"failed to submit draw command buffer!"};

    Swapchain::get().present(frame, renderFinishedSemaphore, windowResized);
}

void Renderer::createCommandPool() {
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex =
        Context::get().getDeviceInfo().queues.graphics.value();

    if (vkCreateCommandPool(Context::get().getDevice(), &createInfo, nullptr,
                            &commandPool) != VK_SUCCESS)
        throw std::runtime_error{"failed to create command pool!"};
}

void Renderer::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(Context::get().getDevice(), &allocInfo,
                                 &commandBuffer) != VK_SUCCESS)
        throw std::runtime_error{"failed to create command buffer!"};
}

void Renderer::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(Context::get().getDevice(), &semaphoreCreateInfo,
                          nullptr, &imageAvailableSemaphore) != VK_SUCCESS)
        throw std::runtime_error{"failed to create sync objects!"};

    if (vkCreateSemaphore(Context::get().getDevice(), &semaphoreCreateInfo,
                          nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
        throw std::runtime_error{"failed to create sync objects!"};

    if (vkCreateFence(Context::get().getDevice(), &fenceCreateInfo, nullptr,
                      &inFlightFence) != VK_SUCCESS)
        throw std::runtime_error{"failed to create sync objects!"};
}

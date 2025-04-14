#include "Renderer.hpp"

#include <vulkan/vulkan_core.h>

#include <glm/mat4x4.hpp>
#include <list>
#include <memory>

#include "BufferManager.hpp"
#include "Context.hpp"
#include "GeometryRenderer.hpp"
#include "Skybox.hpp"
#include "Swapchain.hpp"

using namespace render;

std::unique_ptr<Renderer> Renderer::INSTANCE;

Renderer::Renderer() {
    try {
        createRenderPass();
        framebuffer = Swapchain::get().createFramebuffer(renderPass);

        skyboxRenderer = std::make_unique<SkyboxRenderer>(renderPass);
        geometryRenderer = std::make_unique<GeometryRenderer>(renderPass);
        uiRenderer = std::make_unique<UiRenderer>(renderPass);

        createCommandPool();
        createCommandBuffer();
        createSyncObjects();
    } catch (...) {
        cleanup();
        throw;
    }
}

Renderer::~Renderer() { cleanup(); }

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

    if (skyboxRenderer) {
        skyboxRenderer->cleanup();
        skyboxRenderer.reset();
    }

    if (geometryRenderer) {
        geometryRenderer->cleanup();
        geometryRenderer.reset();
    }

    if (uiRenderer) {
        uiRenderer->cleanup();
        uiRenderer.reset();
    }

    if (renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(Context::get().getDevice(), renderPass, nullptr);
        renderPass = VK_NULL_HANDLE;
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

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = framebuffer->getFrame(frame.index);
    renderPassBeginInfo.renderArea = framebuffer->getRenderArea();

    VkClearValue clearValues[2] = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = framebuffer->getViewport();
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = framebuffer->getRenderArea();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkExtent2D extent = Swapchain::get().getExtent();
    float ratio =
        static_cast<float>(extent.width) / static_cast<float>(extent.height);

    skyboxRenderer->record(commandBuffer, camera, ratio, skybox);
    geometryRenderer->record(commandBuffer, camera, ratio, lights, models);
    uiRenderer->record(commandBuffer, extent, uiModels);

    vkCmdEndRenderPass(commandBuffer);
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

void Renderer::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format =
        Context::get().getDeviceInfo().surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = Context::get().getDeviceInfo().depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.pAttachments = attachments;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    if (vkCreateRenderPass(Context::get().getDevice(), &renderPassCreateInfo,
                           nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error{"failed to create render pass!"};
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

#include "Swapchain.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

using namespace render;

Swapchain::Swapchain(std::shared_ptr<Context> ctx, VkRenderPass renderPass)
    : ctx{std::move(ctx)}, renderPass{renderPass} {
    try {
        createSwapchain();
        createImageViews();
        createFramebuffers();
    } catch (...) {
        cleanup();
        throw;
    }
}

void Swapchain::recreate() {
    waitForRecreateReady();

    for (auto framebuffer : framebuffers)
        if (framebuffer != VK_NULL_HANDLE)
            vkDestroyFramebuffer(ctx->getDevice(), framebuffer, nullptr);

    for (auto imageView : colorImageViews)
        if (imageView != VK_NULL_HANDLE)
            vkDestroyImageView(ctx->getDevice(), imageView, nullptr);

    framebuffers.resize(0);
    colorImageViews.resize(0);

    createSwapchain();
    createImageViews();
    createFramebuffers();
}

void Swapchain::cleanup() {
    for (auto framebuffer : framebuffers)
        if (framebuffer != VK_NULL_HANDLE)
            vkDestroyFramebuffer(ctx->getDevice(), framebuffer, nullptr);

    for (auto imageView : colorImageViews)
        if (imageView != VK_NULL_HANDLE)
            vkDestroyImageView(ctx->getDevice(), imageView, nullptr);

    if (swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(ctx->getDevice(), swapchain, nullptr);
}

Swapchain::Frame Swapchain::acquireFrame(VkSemaphore semaphore) {
    uint32_t index;
    VkResult result =
        vkAcquireNextImageKHR(ctx->getDevice(), swapchain, UINT64_MAX,
                              semaphore, VK_NULL_HANDLE, &index);

    while (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate();

        result = vkAcquireNextImageKHR(ctx->getDevice(), swapchain, UINT64_MAX,
                                       semaphore, VK_NULL_HANDLE, &index);
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error{"failed to acquire swapchain image!"};

    return {index, framebuffers[index]};
}

void Swapchain::present(const Frame &frame, VkSemaphore semaphore,
                        bool forceRecreate) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &semaphore;
    presentInfo.pResults = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &frame.index;

    VkResult result = vkQueuePresentKHR(ctx->getPresentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        recreate();
    else if (result != VK_SUCCESS)
        throw std::runtime_error{"failed to present swapchain image!"};

    if (forceRecreate) recreate();
}

void Swapchain::createSwapchain() {
    Shape shape = getSwapchainShape();
    VkSwapchainKHR oldSwapchain = swapchain;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = ctx->getSurface();
    createInfo.minImageCount = shape.minImageCount;
    createInfo.imageFormat = ctx->getDeviceInfo().surfaceFormat.format;
    createInfo.imageColorSpace = ctx->getDeviceInfo().surfaceFormat.colorSpace;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageExtent = shape.extent;
    createInfo.preTransform = shape.preTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = ctx->getDeviceInfo().presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    const uint32_t queueIndices[] = {
        ctx->getDeviceInfo().queues.graphics.value(),
        ctx->getDeviceInfo().queues.present.value()};

    if (ctx->getDeviceInfo().queues.hasDedicatedPresentQueue()) {
        // The swapchain will be shared between submit queues
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueIndices;
    } else {
        // Present and graphics are the same submit queue, enable exclusive mode
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    if (vkCreateSwapchainKHR(ctx->getDevice(), &createInfo, nullptr,
                             &swapchain) != VK_SUCCESS)
        throw std::runtime_error{"failed to create swapchain!"};

    if (oldSwapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(ctx->getDevice(), oldSwapchain, nullptr);

    extent = createInfo.imageExtent;
    colorFormat = createInfo.imageFormat;
}

void Swapchain::createImageViews() {
    // Destroy old colorImageViews if there are any
    for (auto imageView : colorImageViews)
        if (imageView != VK_NULL_HANDLE)
            vkDestroyImageView(ctx->getDevice(), imageView, nullptr);

    // Obtain the new ones
    vkGetSwapchainImagesKHR(ctx->getDevice(), swapchain, &imageCount, nullptr);

    colorImages.resize(imageCount, VK_NULL_HANDLE);
    vkGetSwapchainImagesKHR(ctx->getDevice(), swapchain, &imageCount,
                            colorImages.data());

    colorImageViews.resize(imageCount, VK_NULL_HANDLE);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = colorImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = colorFormat;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(ctx->getDevice(), &createInfo, nullptr,
                              &colorImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create texture image view!");
    }
}

void Swapchain::createFramebuffers() {
    framebuffers.resize(imageCount, VK_NULL_HANDLE);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageView attachments[] = {colorImageViews[i]};

        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = renderPass;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = attachments;
        createInfo.width = extent.width;
        createInfo.height = extent.height;
        createInfo.layers = 1;

        if (vkCreateFramebuffer(ctx->getDevice(), &createInfo, nullptr,
                                &framebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error{"failed to create framebuffer!"};
    }
}

void Swapchain::waitForRecreateReady() {
    // First wait for the window to actually be ready, GLFW reports 0 size when
    // it detects the window to be minimized.
    int width = 0, height = 0;
    glfwGetFramebufferSize(ctx->getWindow(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(ctx->getWindow(), &width, &height);
        glfwWaitEvents();
    }

    // Then wait for the device to be free
    vkDeviceWaitIdle(ctx->getDevice());
}

Swapchain::Shape Swapchain::getSwapchainShape() {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->getDeviceInfo().device,
                                              ctx->getSurface(), &capabilities);

    uint32_t minImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 &&
        minImageCount > capabilities.maxImageCount)
        minImageCount = capabilities.maxImageCount;

    VkExtent2D extent;
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        extent = capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(ctx->getWindow(), &width, &height);

        extent.width = std::clamp(static_cast<uint32_t>(width),
                                  capabilities.minImageExtent.width,
                                  capabilities.maxImageExtent.width);
        extent.height = std::clamp(static_cast<uint32_t>(height),
                                   capabilities.minImageExtent.height,
                                   capabilities.maxImageExtent.height);
    }

    VkSurfaceTransformFlagBitsKHR preTransform = capabilities.currentTransform;

    return {extent, minImageCount, capabilities.currentTransform};
}

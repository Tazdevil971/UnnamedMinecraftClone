#include "Swapchain.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace render;

std::unique_ptr<Swapchain> Swapchain::INSTANCE;

Swapchain::Swapchain() {
    try {
        createSwapchain();
    } catch (...) {
        cleanup();
        throw;
    }
}

Swapchain::~Swapchain() { cleanup(); }

void Swapchain::recreate() {
    waitForRecreateReady();

    createSwapchain();

    if (framebuffer) framebuffer->recreate(swapchain, extent, colorFormat);
}

void Swapchain::cleanup() {
    if (swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(Context::get().getDevice(), swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
    }

    if (framebuffer) {
        framebuffer->cleanup();
        framebuffer.reset();
    }
}

Swapchain::Frame Swapchain::acquireFrame(VkSemaphore semaphore) {
    uint32_t index;
    VkResult result =
        vkAcquireNextImageKHR(Context::get().getDevice(), swapchain, UINT64_MAX,
                              semaphore, VK_NULL_HANDLE, &index);

    while (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate();

        result = vkAcquireNextImageKHR(Context::get().getDevice(), swapchain,
                                       UINT64_MAX, semaphore, VK_NULL_HANDLE,
                                       &index);
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error{"failed to acquire swapchain image!"};

    return {index};
}

std::shared_ptr<Framebuffer> Swapchain::createFramebuffer(
    VkRenderPass renderPass) {
    assert(!framebuffer);

    framebuffer = std::shared_ptr<Framebuffer>(
        new Framebuffer(swapchain, extent, colorFormat, renderPass));
    return framebuffer;
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

    VkResult result =
        vkQueuePresentKHR(Context::get().getPresentQueue(), &presentInfo);
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
    createInfo.surface = Context::get().getSurface();
    createInfo.minImageCount = shape.minImageCount;
    createInfo.imageFormat =
        Context::get().getDeviceInfo().surfaceFormat.format;
    createInfo.imageColorSpace =
        Context::get().getDeviceInfo().surfaceFormat.colorSpace;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageExtent = shape.extent;
    createInfo.preTransform = shape.preTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = Context::get().getDeviceInfo().presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    const uint32_t queueIndices[] = {
        Context::get().getDeviceInfo().queues.graphics.value(),
        Context::get().getDeviceInfo().queues.present.value()};

    if (Context::get().getDeviceInfo().queues.hasDedicatedPresentQueue()) {
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

    if (vkCreateSwapchainKHR(Context::get().getDevice(), &createInfo, nullptr,
                             &swapchain) != VK_SUCCESS)
        throw std::runtime_error{"failed to create swapchain!"};

    if (oldSwapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(Context::get().getDevice(), oldSwapchain,
                              nullptr);

    extent = createInfo.imageExtent;
    colorFormat = createInfo.imageFormat;
}

void Swapchain::waitForRecreateReady() {
    // First wait for the window to actually be ready, GLFW reports 0 size when
    // it detects the window to be minimized.
    int width = 0, height = 0;
    glfwGetFramebufferSize(Context::get().getWindow(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(Context::get().getWindow(), &width, &height);
        glfwWaitEvents();
    }

    // Then wait for the device to be free
    vkDeviceWaitIdle(Context::get().getDevice());
}

Swapchain::Shape Swapchain::getSwapchainShape() {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        Context::get().getDeviceInfo().device, Context::get().getSurface(),
        &capabilities);

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
        glfwGetFramebufferSize(Context::get().getWindow(), &width, &height);

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

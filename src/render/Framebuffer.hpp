#pragma once

#include <vulkan/vulkan.h>

#include <cassert>

#include "Primitives.hpp"

namespace render {

class Swapchain;

class Framebuffer {
    friend class render::Swapchain;

public:
    VkFramebuffer getFrame(uint32_t idx) const {
        assert(idx < imageCount);
        return framebuffers[idx];
    }

    VkExtent2D getExtent() const { return extent; }
    VkFormat getColorFormat() const { return colorFormat; }

    VkViewport getViewport() const {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        return viewport;
    }

    VkRect2D getScissor() const {
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = extent;

        return scissor;
    }

private:
    Framebuffer(VkSwapchainKHR swapchain, VkExtent2D extent,
                VkFormat colorFormat, VkRenderPass renderPass);

    void recreate(VkSwapchainKHR swapchain, VkExtent2D extent,
                  VkFormat colorFormat);
    void cleanup();

    void getImageCount(VkSwapchainKHR swapchain);
    void createColorImages(VkSwapchainKHR swapchain);
    void createDepthImages();
    void createFramebuffers();

    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkExtent2D extent;
    VkFormat colorFormat;

    uint32_t imageCount{0};
    std::vector<Image> colorImages;
    std::vector<Image> depthImages;
    std::vector<VkFramebuffer> framebuffers;
};

}  // namespace render
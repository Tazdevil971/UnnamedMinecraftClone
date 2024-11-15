#pragma once

#include <memory>

#include "Context.hpp"

namespace render {

class Swapchain {
   public:
    struct Frame {
        uint32_t index;
        VkFramebuffer framebuffer;
    };

    static std::shared_ptr<Swapchain> create(std::shared_ptr<Context> ctx,
                                             VkRenderPass renderPass) {
        return std::make_shared<Swapchain>(ctx, renderPass);
    }

    Swapchain(std::shared_ptr<Context> ctx, VkRenderPass renderPass);

    void recreate();
    void cleanup();

    Frame acquireFrame(VkSemaphore semaphore);
    void present(const Frame &frame, VkSemaphore semaphore,
                 bool forceRecreate = false);

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

    VkRect2D getRenderArea() const {
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = extent;

        return scissor;
    }

   private:
    struct Shape {
        VkExtent2D extent;
        uint32_t minImageCount;
        VkSurfaceTransformFlagBitsKHR preTransform;
    };

    void createSwapchain();
    void createImageViews();
    void createFramebuffers();

    void waitForRecreateReady();
    Shape getSwapchainShape();

    std::shared_ptr<Context> ctx;
    VkRenderPass renderPass{VK_NULL_HANDLE};

    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    VkExtent2D extent;
    VkFormat colorFormat;

    uint32_t imageCount{0};
    std::vector<VkImage> colorImages;
    std::vector<VkImageView> colorImageViews;
    std::vector<VkFramebuffer> framebuffers;
};

}  // namespace render
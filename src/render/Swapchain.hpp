#pragma once

#include <memory>

#include "BufferManager.hpp"
#include "Context.hpp"

namespace render {

class Swapchain {
   public:
    struct Frame {
        uint32_t index;
        VkFramebuffer framebuffer;
    };

    static std::shared_ptr<Swapchain> create(
        std::shared_ptr<Context> ctx, std::shared_ptr<BufferManager> bufferMgr,
        VkRenderPass renderPass) {
        return std::make_shared<Swapchain>(std::move(ctx), std::move(bufferMgr),
                                           renderPass);
    }

    Swapchain(std::shared_ptr<Context> ctx,
              std::shared_ptr<BufferManager> bufferMgr,
              VkRenderPass renderPass);

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
    void createColorImages();
    void createDepthImages();
    void createFramebuffers();

    void waitForRecreateReady();
    Shape getSwapchainShape();

    std::shared_ptr<Context> ctx;
    std::shared_ptr<BufferManager> bufferMgr;

    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    VkRenderPass renderPass;
    VkExtent2D extent;
    VkFormat colorFormat;

    uint32_t imageCount{0};
    std::vector<SimpleImage> colorImages;
    std::vector<DepthImage> depthImages;
    std::vector<VkFramebuffer> framebuffers;
};

}  // namespace render
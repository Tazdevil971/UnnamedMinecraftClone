#pragma once

#include <memory>

#include "BufferManager.hpp"
#include "Context.hpp"
#include "Framebuffer.hpp"

namespace render {

class Swapchain {
   public:
    struct Frame {
        uint32_t index;
    };

    static std::shared_ptr<Swapchain> create(
        std::shared_ptr<Context> ctx,
        std::shared_ptr<BufferManager> bufferMgr) {
        return std::make_shared<Swapchain>(std::move(ctx),
                                           std::move(bufferMgr));
    }

    Swapchain(std::shared_ptr<Context> ctx,
              std::shared_ptr<BufferManager> bufferMgr);

    void recreate();
    void cleanup();

    Frame acquireFrame(VkSemaphore semaphore);
    void present(const Frame &frame, VkSemaphore semaphore,
                 bool forceRecreate = false);

    VkExtent2D getExtent() const { return extent; }
    VkFormat getColorFormat() const { return colorFormat; }

    std::shared_ptr<Framebuffer> createFramebuffer(VkRenderPass renderPass);

   private:
    struct Shape {
        VkExtent2D extent;
        uint32_t minImageCount;
        VkSurfaceTransformFlagBitsKHR preTransform;
    };

    void createSwapchain();

    void waitForRecreateReady();
    Shape getSwapchainShape();

    std::shared_ptr<Context> ctx;
    std::shared_ptr<BufferManager> bufferMgr;
    std::shared_ptr<Framebuffer> framebuffer;

    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    VkExtent2D extent;
    VkFormat colorFormat;
};

}  // namespace render
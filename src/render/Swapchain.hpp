#pragma once

#include <memory>
#include <stdexcept>

#include "Framebuffer.hpp"
#include "Managed.hpp"

namespace render {

class Swapchain {
private:
    static std::unique_ptr<Swapchain> INSTANCE;

public:
    static void create() { INSTANCE.reset(new Swapchain()); }

    static Swapchain& get() {
        if (INSTANCE) {
            return *INSTANCE;
        } else {
            throw std::runtime_error{"Swapchain not yet created"};
        }
    }

    static void destroy() { INSTANCE.reset(); }

    struct Frame {
        uint32_t index;
    };

    void recreate();

    Frame acquireFrame(VkSemaphore semaphore);
    void present(const Frame& frame, VkSemaphore semaphore,
                 bool forceRecreate = false);

    VkExtent2D getExtent() const { return extent; }
    VkFormat getColorFormat() const { return colorFormat; }

    std::shared_ptr<Framebuffer> createFramebuffer(VkRenderPass renderPass);

private:
    Swapchain();

    struct Shape {
        VkExtent2D extent;
        uint32_t minImageCount;
        VkSurfaceTransformFlagBitsKHR preTransform;
    };

    void createSwapchain();

    void waitForRecreateReady();
    Shape getSwapchainShape();

    std::shared_ptr<Framebuffer> framebuffer;

    ManagedSwapchain swapchain;
    VkExtent2D extent;
    VkFormat colorFormat;
};

}  // namespace render
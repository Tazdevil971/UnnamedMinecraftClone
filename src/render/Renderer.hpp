#pragma once

#include <list>

#include "Context.hpp"
#include "Swapchain.hpp"
#include "TextureManager.hpp"

namespace render {

class Renderer {
   private:
    static std::unique_ptr<Renderer> INSTANCE;

   public:
    static void create() { INSTANCE.reset(new Renderer()); }

    static Renderer& get() {
        if (INSTANCE) {
            return *INSTANCE;
        } else {
            throw std::runtime_error{"Renderer not yet created"};
        }
    }

    static void destroy() { INSTANCE.reset(); }

    ~Renderer();

    void render(const Camera& camera, std::list<GeometryModel> models,
                bool windowResized);

   private:
    Renderer();

    void cleanup();

    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};

    VkCommandPool commandPool{VK_NULL_HANDLE};

    std::shared_ptr<Framebuffer> framebuffer;

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence inFlightFence{VK_NULL_HANDLE};

    void createRenderPass();
    void createGraphicsPipeline();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();
    void recordGeometryModelRender(const GeometryModel& model, glm::mat4 vp);
};
}  // namespace render
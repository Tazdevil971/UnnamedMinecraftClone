#pragma once

#include "Context.hpp"
#include "Swapchain.hpp"
#include "TextureManager.hpp"

#include <list>

namespace render {

class Renderer {
   public:
    static std::shared_ptr<Renderer> create(
        std::shared_ptr<Context> ctx,
        std::shared_ptr<TextureManager> textureMgr,
        std::shared_ptr<BufferManager> bufferMgr) {
        return std::make_shared<Renderer>(std::move(ctx), std::move(textureMgr),
                                          std::move(bufferMgr));
    }

    Renderer(std::shared_ptr<Context> ctx,
             std::shared_ptr<TextureManager> textureMgr,
             std::shared_ptr<BufferManager> bufferMgr);
    void render(const Camera &camera, std::list<SimpleModel> models,
                bool windowResized);
    void cleanup();



   private:
    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};

    VkCommandPool commandPool{VK_NULL_HANDLE};

    std::shared_ptr<Context> ctx;
    std::shared_ptr<Swapchain> swapchain;
    std::shared_ptr<TextureManager> textureMgr;

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence inFlightFence{VK_NULL_HANDLE};

    void createRenderPass();
    void createGraphicsPipeline();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();
    void recordCommandBuffer(VkFramebuffer framebuffer,
                             const SimpleModel& model,
                             glm::mat4 viewProjection);
};
}  // namespace render
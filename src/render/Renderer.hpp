#pragma once

#include "Context.hpp"
#include "Swapchain.hpp"
#include "TextureManager.hpp"


namespace render {

class Renderer {
   public:
    Renderer(std::shared_ptr<Context> ctx,
             std::shared_ptr<TextureManager> textureMgr,
             std::shared_ptr<BufferManager> bufferMgr);
    void render(std::list<SimpleModel> models, bool windowResized);
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
    void createSyncObjects();
    void recordCommandBuffer(VkFramebuffer framebuffer, const SimpleModel & model);

};
}  // namespace 
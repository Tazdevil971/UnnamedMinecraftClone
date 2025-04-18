#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <list>

#include "Primitives.hpp"

namespace render {

class UiRenderer {
public:
    UiRenderer(VkRenderPass renderPass);

    void cleanup();

    void record(VkCommandBuffer commandBuffer, VkExtent2D extent,
                std::list<UiModel> models);

private:
    struct PushBuffer {
        glm::vec2 pos;
        glm::vec2 dimension;
    };

    void recordSingle(VkCommandBuffer commandBuffer, VkExtent2D extent,
                      const UiModel& model);

    void createPipeline(VkRenderPass renderPass);

    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};
};

}  // namespace render
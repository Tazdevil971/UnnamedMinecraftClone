#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <list>

#include "Managed.hpp"
#include "Primitives.hpp"

namespace render {

class UiRenderer {
public:
    UiRenderer(VkRenderPass renderPass);

    void record(VkCommandBuffer commandBuffer, VkExtent2D extent,
                std::list<UiModel> models);

private:
    struct PushBuffer {
        glm::vec2 pos;
        glm::vec2 dimension;
        glm::vec2 anchorPoint;
    };

    void cleanup();

    void recordSingle(VkCommandBuffer commandBuffer, VkExtent2D extent,
                      const UiModel& model);

    void createPipeline(VkRenderPass renderPass);

    ManagedPipelineLayout pipelineLayout;
    ManagedPipeline pipeline;
};

}  // namespace render
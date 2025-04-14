#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Primitives.hpp"
#include "Skybox.hpp"

namespace render {

class SkyboxRenderer {
public:
    SkyboxRenderer(VkRenderPass renderPass);

    void cleanup();

    void record(VkCommandBuffer commandBuffer, const Camera& camera,
                float ratio, const Skybox& skybox);

private:
    struct PushBuffer {
        glm::mat4 mvp;
    };

    void createPipeline(VkRenderPass renderPass);

    Ubo skyboxInfoUbo;

    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};
};

}  // namespace render
#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <list>

#include "Primitives.hpp"

namespace render {

class GeometryRenderer {
public:
    GeometryRenderer(VkRenderPass renderPass);

    void cleanup();

    struct LightInfo {
        glm::vec3 ambientColor;
        glm::vec3 sunDir;
        glm::vec3 sunColor;
    };

    void record(VkCommandBuffer commandBuffer, const Camera& camera,
                float ratio, const LightInfo& lights, Texture depthTexture,
                std::list<GeometryModel> models);

private:
    struct PushBuffer {
        glm::mat4 m;
        glm::mat4 vp;
    };

    struct LightInfoUbo {
        glm::mat4 shadowVP;
        glm::vec4 ambientColor;
        glm::vec4 sunDir;
        glm::vec4 sunColor;
    };

    void recordSingle(VkCommandBuffer commandBuffer, glm::mat4 vp,
                      Texture depthTexture, const GeometryModel& model);

    void createPipeline(VkRenderPass renderPass);

    Ubo lightInfoUbo;

    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};
};

}  // namespace render
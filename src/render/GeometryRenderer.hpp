#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <list>

#include "Managed.hpp"
#include "Primitives.hpp"

namespace render {

class GeometryRenderer {
public:
    GeometryRenderer(VkRenderPass renderPass);

    struct LightInfo {
        glm::vec3 ambientColor;
        glm::vec3 sunDir;
        glm::vec3 sunColor;
    };

    void record(VkCommandBuffer commandBuffer, const Camera& camera,
                float ratio, const LightInfo& lights,
                const Texture& depthTexture, std::list<GeometryModel> models);

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
        glm::vec4 viewPos;
    };

    void recordSingle(VkCommandBuffer commandBuffer, glm::mat4 vp,
                      const Texture& depthTexture, const GeometryModel& model);

    void createPipeline(VkRenderPass renderPass);

    Ubo lightInfoUbo;

    ManagedPipelineLayout pipelineLayout;
    ManagedPipeline pipeline;
};

}  // namespace render
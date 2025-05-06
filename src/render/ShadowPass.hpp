#pragma once

#include <vulkan/vulkan_core.h>

#include <list>

#include "Managed.hpp"
#include "Primitives.hpp"

namespace render {

class ShadowPass {
public:
    const VkExtent2D SHADOWMAP_EXTENT{2048, 2048};

    ShadowPass();
    ~ShadowPass();

    void record(VkCommandBuffer commandBuffer, const Camera& camera,
                glm::vec3 lightDir, std::list<GeometryModel> models);

    Texture getDepthTexture() const { return depthTexture; }

    static glm::mat4 computeShadowVP(glm::vec3 center, glm::vec3 lightDir);

private:
    struct PushBuffer {
        glm::mat4 mvp;
    };

    void cleanup();

    VkViewport getViewport() const {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(SHADOWMAP_EXTENT.width);
        viewport.height = static_cast<float>(SHADOWMAP_EXTENT.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        return viewport;
    }

    VkRect2D getScissor() const {
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = SHADOWMAP_EXTENT;

        return scissor;
    }

    void recordSingle(VkCommandBuffer commandBuffer, glm::mat4 vp,
                      const GeometryModel& model);

    void createRenderPass();
    void createFramebuffer();
    void createPipeline();

    Texture depthTexture;

    ManagedRenderPass renderPass;
    ManagedFramebuffer framebuffer;
    ManagedPipelineLayout pipelineLayout;
    ManagedPipeline pipeline;
};

}  // namespace render
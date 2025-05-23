#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

#include "Framebuffer.hpp"
#include "GeometryRenderer.hpp"
#include "Managed.hpp"
#include "SkyboxRenderer.hpp"
#include "Swapchain.hpp"
#include "UiRenderer.hpp"

namespace render {

class ForwardPass {
public:
    ForwardPass();

    void record(VkCommandBuffer commandBuffer, Swapchain::Frame frame,
                const Camera& camera, const Skybox& skybox,
                const GeometryRenderer::LightInfo& lights,
                const Texture& depthTexture, std::list<GeometryModel> models,
                std::list<UiModel> uiModels);

private:
    void createRenderPass();

    ManagedRenderPass renderPass;

    std::shared_ptr<Framebuffer> framebuffer;

    std::unique_ptr<SkyboxRenderer> skyboxRenderer;
    std::unique_ptr<GeometryRenderer> geometryRenderer;
    std::unique_ptr<UiRenderer> uiRenderer;
};

}  // namespace render
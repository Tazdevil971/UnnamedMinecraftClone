#pragma once

#include <vulkan/vulkan.h>

#include <list>
#include <memory>

#include "ForwardPass.hpp"
#include "Managed.hpp"
#include "Primitives.hpp"
#include "ShadowPass.hpp"
#include "Skybox.hpp"

namespace render {

class Renderer {
public:
    Renderer();

    static std::unique_ptr<Renderer> create() {
        return std::make_unique<Renderer>();
    }

    void render(const Camera& camera, const Skybox& skybox,
                const GeometryRenderer::LightInfo& lights,
                std::list<GeometryModel> models, std::list<UiModel> uiModels,
                bool windowResized);

    const Texture& getDepthTexture() const {
        return shadowPass->getDepthTexture();
    }

private:
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();

    ManagedCommandPool commandPool;
    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};

    std::unique_ptr<ShadowPass> shadowPass;
    std::unique_ptr<ForwardPass> forwardPass;

    ManagedSemaphore imageAvailableSemaphore;
    ManagedSemaphore renderFinishedSemaphore;
    ManagedFence inFlightFence;
};
}  // namespace render
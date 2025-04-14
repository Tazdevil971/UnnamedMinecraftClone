#pragma once

#include <vulkan/vulkan.h>

#include <list>
#include <memory>

#include "ForwardPass.hpp"
#include "Primitives.hpp"
#include "Skybox.hpp"


namespace render {

class Renderer {
public:
    Renderer();

    void cleanup();

    void render(const Camera& camera, const Skybox& skybox,
                const GeometryRenderer::LightInfo& lights,
                std::list<GeometryModel> models, std::list<UiModel> uiModels,
                bool windowResized);

private:
    VkCommandPool commandPool{VK_NULL_HANDLE};

    std::unique_ptr<ForwardPass> forwardPass;

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence inFlightFence{VK_NULL_HANDLE};

    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();
};
}  // namespace render
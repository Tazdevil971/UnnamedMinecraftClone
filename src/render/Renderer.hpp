#pragma once

#include <vulkan/vulkan.h>

#include <list>
#include <memory>
#include <stdexcept>

#include "Framebuffer.hpp"
#include "GeometryRenderer.hpp"
#include "Primitives.hpp"
#include "Skybox.hpp"
#include "SkyboxRenderer.hpp"

namespace render {

class Renderer {
private:
    static std::unique_ptr<Renderer> INSTANCE;

public:
    static void create() { INSTANCE.reset(new Renderer()); }

    static Renderer& get() {
        if (INSTANCE) {
            return *INSTANCE;
        } else {
            throw std::runtime_error{"Renderer not yet created"};
        }
    }

    static void destroy() { INSTANCE.reset(); }

    ~Renderer();

    void render(const Camera& camera, const Skybox& skybox,
                const GeometryRenderer::LightInfo& lights,
                std::list<GeometryModel> models, std::list<UiModel> uiModels,
                bool windowResized);

private:
    Renderer();

    void cleanup();

    struct UiPushBuffer {
        glm::vec2 pos;
        glm::vec2 dimension;
    };

    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkPipelineLayout uiPipelineLayout{VK_NULL_HANDLE};
    VkPipeline uiPipeline{VK_NULL_HANDLE};

    VkCommandPool commandPool{VK_NULL_HANDLE};

    std::unique_ptr<GeometryRenderer> geometryRenderer;
    std::unique_ptr<SkyboxRenderer> skyboxRenderer;
    std::shared_ptr<Framebuffer> framebuffer;

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence inFlightFence{VK_NULL_HANDLE};

    void createRenderPass();
    void createUiGraphicsPipeline();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();

    void doUiRender(std::list<UiModel> uiModels);
    void recordUiModelRender(const UiModel& model);
};
}  // namespace render
#pragma once

#include <vulkan/vulkan.h>

#include <list>
#include <memory>
#include <stdexcept>

#include "Framebuffer.hpp"
#include "Primitives.hpp"
#include "Skybox.hpp"

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

    struct LightInfo {
        glm::vec4 ambientColor;
        glm::vec4 sunDir;
        glm::vec4 sunColor;
    };

    void render(const Camera& camera, const Skybox& skybox,
                const LightInfo& lights, std::list<GeometryModel> models,
                std::list<UiModel> uiModels, bool windowResized);

private:
    Renderer();

    void cleanup();

    struct SkyboxPushBuffer {
        glm::mat4 mvp;
    };

    struct GeometryPushBuffer {
        glm::mat4 m;
        glm::mat4 vp;
    };

    Ubo skyboxInfo;
    Ubo geometryLightInfo;

    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkPipelineLayout skyboxPipelineLayout{VK_NULL_HANDLE};
    VkPipeline skyboxPipeline{VK_NULL_HANDLE};
    VkPipelineLayout geometryPipelineLayout{VK_NULL_HANDLE};
    VkPipeline geometryPipeline{VK_NULL_HANDLE};
    VkPipelineLayout uiPipelineLayout{VK_NULL_HANDLE};
    VkPipeline uiPipeline{VK_NULL_HANDLE};

    VkCommandPool commandPool{VK_NULL_HANDLE};

    std::shared_ptr<Framebuffer> framebuffer;

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence inFlightFence{VK_NULL_HANDLE};

    void createRenderPass();
    void createSkyboxGraphicsPipeline();
    void createGeometryGraphicsPipeline();
    void createUiGraphicsPipeline();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();

    void doSkyboxRender(glm::mat4 vp, const Skybox& skybox);
    void doGeometryRender(glm::mat4 vp, const LightInfo& lights,
                          std::list<GeometryModel> models);
    void recordGeometryModelRender(const GeometryModel& model, glm::mat4 vp);
    void doUiRender(std::list<UiModel> uiModels);
    void recordUiModelRender(const UiModel& model);
};
}  // namespace render
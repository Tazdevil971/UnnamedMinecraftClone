#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <fstream>
#include <memory>
#include <optional>
#include <vector>

namespace render {

class Context {
   public:
    struct QueueFamilies {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;

        bool isComplete() const {
            return graphics.has_value() && present.has_value();
        }

        bool hasDedicatedPresentQueue() const { return present != graphics; }

        std::vector<uint32_t> indices() const {
            std::vector<uint32_t> out;
            if (graphics.has_value()) out.push_back(graphics.value());

            if (present.has_value() && hasDedicatedPresentQueue())
                out.push_back(present.value());

            return out;
        }
    };

    VkShaderModule loadShaderModule(const std::string &path) const;
    VkShaderModule loadShaderModule(const uint32_t *code, size_t size) const;

    void waitDeviceIdle();

    struct DeviceInfo {
        QueueFamilies queues;
        VkPhysicalDevice device{VK_NULL_HANDLE};
        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR presentMode;
        VkFormat depthFormat;
        bool hasFilterAnisotropy;
        bool hasKHRDedicatedAllocation;
        float maxSamplerAnisotropy;
    };

    static std::shared_ptr<Context> create(GLFWwindow *window) {
        return std::make_shared<Context>(window);
    }

    Context(GLFWwindow *window);

    void cleanup();

    VkDevice getDevice() const { return device; }
    GLFWwindow *getWindow() const { return window; }
    VkInstance getInstance() const { return instance; }
    VkSurfaceKHR getSurface() const { return surface; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkQueue getPresentQueue() const { return presentQueue; }
    const DeviceInfo &getDeviceInfo() const { return deviceInfo; }

   private:
    struct InstanceExtensions {
        bool hasKHRPortabilityEnumeration;
    };

    struct InstanceLayers {
        bool hasKhronos;
    };

    struct DeviceExtensions {
        bool hasKHRSwapchain;
        bool hasKHRDedicatedAllocation;
    };

    void createInstance();
    void createSurface();
    DeviceInfo pickPhysicalDevice();
    void createDevice();

    InstanceExtensions getInstanceExtensionSupport();
    InstanceLayers getInstanceLayerSupport();
    DeviceExtensions getDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilies getDeviceQueueFamilies(VkPhysicalDevice device);
    std::optional<VkSurfaceFormatKHR> chooseSurfaceFormat(
        VkPhysicalDevice device);
    std::optional<VkPresentModeKHR> choosePresentMode(VkPhysicalDevice device);
    std::optional<VkFormat> findFirstSupportedFormat(
        VkPhysicalDevice device, const std::vector<VkFormat> &formats,
        VkImageTiling tiling, VkFormatFeatureFlags features) const;

    GLFWwindow *window{nullptr};
    VkInstance instance{VK_NULL_HANDLE};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};

    DeviceInfo deviceInfo;
};

}  // namespace render
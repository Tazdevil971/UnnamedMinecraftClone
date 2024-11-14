
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>
#include <vector>

namespace render {

class Context {
   public:
    struct QueueFamilies {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;
        std::optional<uint32_t> transfer;

        bool isComplete() const {
            return graphics.has_value() && present.has_value() &&
                   transfer.has_value();
        }

        bool hasDedicatedTransferQueue() const { return transfer != graphics; }

        bool hasDedicatedPresentQueue() const { return present != graphics; }
    
        std::vector<uint32_t> indices() const {
            std::vector<uint32_t> out;
            if (graphics.has_value()) out.push_back(graphics.value());

            if (present.has_value() && hasDedicatedPresentQueue())
                out.push_back(present.value());

            if (transfer.has_value() && hasDedicatedTransferQueue())
                out.push_back(transfer.value());

            return out;
        }
    };

    struct DeviceInfo {
        QueueFamilies queues;
        VkPhysicalDevice device{VK_NULL_HANDLE};
        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR presentMode;
        bool hasFilterAnisotropy;
    };

    Context(GLFWwindow *window);

    VkDevice getDevice() const {
        return device;
    }

    VkInstance getInstance() const {
        return instance;
    }

    VkSurfaceKHR getSurface() const {
        return surface;
    }

    VkQueue getGraphicsQueue() const {
        return graphicsQueue;
    }

    VkQueue getPresentQueue() const {
        return presentQueue;
    }

    VkQueue getTransferQueue() const {
        return transferQueue;
    }

    const DeviceInfo &getDeviceInfo() const {
        return deviceInfo;
    }

    void cleanup();

   private:
    struct InstanceExtensionSupport {
        bool hasKHRPortabilityEnumeration;
    };

    struct InstanceLayerSupport {
        bool hasKhronos;
    };

    struct DeviceExtensionSupport {
        bool hasKHRSwapchain;
    };

    void createInstance();
    void createSurface();
    DeviceInfo pickPhysicalDevice();
    void createDevice();

    InstanceExtensionSupport getInstanceExtensionSupport();
    InstanceLayerSupport getInstanceLayerSupport();
    DeviceExtensionSupport getDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilies getDeviceQueueFamilies(VkPhysicalDevice device);
    std::optional<VkSurfaceFormatKHR> chooseSurfaceFormat(
        VkPhysicalDevice device);
    std::optional<VkPresentModeKHR> choosePresentMode(VkPhysicalDevice device);

    GLFWwindow *window{nullptr};
    VkInstance instance{VK_NULL_HANDLE};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};
    VkQueue transferQueue{VK_NULL_HANDLE};

    DeviceInfo deviceInfo;
};

}  // namespace render
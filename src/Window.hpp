#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

class Window {
   public:
    Window();

    void init();
    void mainLoop();
    void cleanup();

    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription description{};
            description.binding = 0;
            description.stride = sizeof(Vertex);
            description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return description;
        }

        static std::array<VkVertexInputAttributeDescription, 3>
        getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 3> descriptions{};
            descriptions[0].binding = 0;
            descriptions[0].location = 0;
            descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            descriptions[0].offset = offsetof(Vertex, pos);

            descriptions[1].binding = 0;
            descriptions[1].location = 1;
            descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            descriptions[1].offset = offsetof(Vertex, color);

            descriptions[2].binding = 0;
            descriptions[2].location = 2;
            descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            descriptions[2].offset = offsetof(Vertex, texCoord);

            return descriptions;
        }
    };

    struct Ubo {
        glm::mat4 mvp;
    };

   private:
    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    struct VkInstanceExtensionSupport {
        bool hasKHRPortabilityEnumeration;
    };

    struct VkInstanceLayerSupport {
        bool hasKhronos;
    };

    struct VkDeviceExtensionSupport {
        bool hasKHRSwapchain;
    };

    struct QueueFamilies {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;

        bool isComplete() {
            return graphics.has_value() && present.has_value();
        }

        std::vector<uint32_t> indices() {
            std::vector<uint32_t> res;
            if (graphics.has_value()) res.push_back(graphics.value());

            if (present.has_value() && present != graphics)
                res.push_back(present.value());

            return res;
        }
    };

    struct PhysicalDevice {
        VkPhysicalDevice handle{VK_NULL_HANDLE};
        QueueFamilies queues;
        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR presentMode;
    };

    void initWindow();
    void initVulkan();

    void createVkInstance();
    void createSurface();
    void createDevice();
    void createSwapChain();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createTextureImage();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffer();
    void createSyncObjects();

    void drawFrame();
    void recordCommandBuffer(VkCommandBuffer commandBuffer,
                             uint32_t imageIndex);

    void cleanupSwapchain();
    void recreateSwapchain();

    std::vector<const char*> getGlfwRequiredInstanceExtensions();
    VkInstanceExtensionSupport getVkInstanceExtensionSupport();
    VkInstanceLayerSupport getVkInstanceLayerSupport();
    VkDeviceExtensionSupport getVkDeviceExtensionSupport(
        VkPhysicalDevice device);
    QueueFamilies getVkDeviceQueueFamilies(VkPhysicalDevice device);
    std::optional<VkSurfaceFormatKHR> chooseSurfaceFormat(
        VkPhysicalDevice device);
    std::optional<VkPresentModeKHR> choosePresentMode(VkPhysicalDevice device);
    PhysicalDevice pickPhysicalDevice();

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer& buffer,
                      VkDeviceMemory& deviceMemory);
    void createImage(uint32_t width, uint32_t height, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image,
                     VkDeviceMemory& imageMemory);
    void createImageView(VkImage image, VkFormat format, VkImageView& view);

    void transitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                           uint32_t height);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t findMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    VkShaderModule loadShaderModule(const std::string& path);

    float getTime();
    glm::mat4 createMVP(float time);

    void onResize(int width, int height);

    static void glfwOnResizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* window{nullptr};
    bool windowResized{false};

    PhysicalDevice physicalDevice;

    VkInstance instance{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};

    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    VkExtent2D swapchainExtent;
    VkFormat swapchainFormat;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainFramebuffers;

    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPipeline pipeline{VK_NULL_HANDLE};

    VkCommandPool commandPool{VK_NULL_HANDLE};

    uint32_t currentFrame{0};
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
    std::vector<VkDescriptorSet> descriptorSets;

    VkBuffer vertexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory vertexBufferMemory{VK_NULL_HANDLE};
    VkBuffer indexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory indexBufferMemory{VK_NULL_HANDLE};

    VkImage textureImage{VK_NULL_HANDLE};
    VkDeviceMemory textureImageMemory{VK_NULL_HANDLE};
    VkImageView textureImageView{VK_NULL_HANDLE};
    VkSampler textureSampler{VK_NULL_HANDLE};

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkSurfaceKHR surface{VK_NULL_HANDLE};
};
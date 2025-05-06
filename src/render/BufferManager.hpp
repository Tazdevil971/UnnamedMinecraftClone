#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Managed.hpp"

namespace render {

struct Ubo;
struct UboDescriptorSet;
struct BaseMesh;
struct Image;
struct Texture;
struct TextureDescriptorSet;

class BufferManager {
    friend class UboDescriptorSet;
    friend class TextureDescriptorSet;

private:
    static std::unique_ptr<BufferManager> INSTANCE;

public:
    static void create(size_t uboPoolSize, size_t texturePoolSize) {
        INSTANCE.reset(new BufferManager(uboPoolSize, texturePoolSize));
    }

    static BufferManager& get() {
        if (INSTANCE) {
            return *INSTANCE;
        } else {
            throw std::runtime_error{"BufferManager not yet created"};
        }
    }

    static void destroy() { INSTANCE.reset(); }

    void performDeferOps();

    // Mesh stuff
    template <typename T>
    T allocateMesh(const std::vector<uint16_t>& indices,
                   const std::vector<typename T::Vertex>& vertices);

    void deallocateMeshDefer(BaseMesh&& mesh);

    // UBO stuff
    VkDescriptorSetLayout getUboLayout() const { return *uboLayout; }

    Ubo allocateUbo(size_t size);

    // Image stuff
    Image allocateDepthImage(uint32_t width, uint32_t height);

    Image importImage(VkImage image, uint32_t width, uint32_t height,
                      VkFormat format);
    Image allocateImage(const uint8_t* pixels, uint32_t width, uint32_t height,
                        VkFormat format);
    Image allocateImage(const std::string& path, VkFormat format);

    // Texture stuff
    VkDescriptorSetLayout getTextureLayout() const { return *textureLayout; }

    Texture allocateTexture(const std::string& path, VkFormat format);
    Texture allocateDepthTexture(uint32_t width, uint32_t height);

private:
    BufferManager(size_t uboPoolSize, size_t texturePoolSize);

    BaseMesh allocateMeshInner(const void* indicesData, size_t indicesDataSize,
                               size_t indicesCount, const void* vertexData,
                               size_t vertexDataSize, size_t vertexCount);

    void releaseUboDescriptorSet(VkDescriptorSet descriptor);
    void releaseTextureDescriptorSet(VkDescriptorSet descriptor);

    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();

    void createUboLayout();
    void createUboDescriptorPool(uint32_t size);
    void createUboDescriptorSets(uint32_t size);

    void createTextureLayout();
    void createTextureDescriptorPool(uint32_t size);
    void createTextureDescriptorSets(uint32_t size);

    ManagedBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                               VmaMemoryUsage vmaUsage,
                               VmaAllocationCreateFlags vmaFlags);

    ManagedImage createImage(uint32_t width, uint32_t height, VkFormat format,
                             VkImageUsageFlags usage, VmaMemoryUsage vmaUsage,
                             VmaAllocationCreateFlags vmaFlags);

    ManagedImageView createImageView(VkImage image, VkFormat format,
                                     VkImageAspectFlags aspect);

    void startRecording();
    void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
    void copyBufferToImage(VkBuffer src, VkImage dst, uint32_t width,
                           uint32_t height);
    void transitionImageLayout(VkImage image, VkImageLayout oldLayout,
                               VkImageLayout newLayout, VkFormat format);
    void submitAndWait();

    std::vector<BaseMesh> meshDefer;

    std::vector<VkDescriptorSet> textureDescriptorSets;
    ManagedDescriptorSetLayout textureLayout;
    ManagedDescriptorPool textureDescriptorPool;

    std::vector<VkDescriptorSet> uboDescriptorSets;
    ManagedDescriptorSetLayout uboLayout;
    ManagedDescriptorPool uboDescriptorPool;

    ManagedCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    ManagedFence syncFence;
};

template <typename T>
T BufferManager::allocateMesh(const std::vector<uint16_t>& indices,
                              const std::vector<typename T::Vertex>& vertices) {
    return T{allocateMeshInner(
        indices.data(), indices.size() * sizeof(uint16_t), indices.size(),
        vertices.data(), vertices.size() * sizeof(typename T::Vertex),
        vertices.size())};
}

struct UboDescriptorSet {
    VkDescriptorSet inner{VK_NULL_HANDLE};

    UboDescriptorSet() {}
    UboDescriptorSet(const UboDescriptorSet&) = delete;
    UboDescriptorSet(UboDescriptorSet&& other) {
        inner = other.inner;
        other.inner = VK_NULL_HANDLE;
    }

    ~UboDescriptorSet() { BufferManager::get().releaseUboDescriptorSet(inner); }
};

struct TextureDescriptorSet {
    VkDescriptorSet inner{VK_NULL_HANDLE};

    TextureDescriptorSet() {}
    TextureDescriptorSet(const TextureDescriptorSet&) = delete;
    TextureDescriptorSet(TextureDescriptorSet&& other) {
        inner = other.inner;
        other.inner = VK_NULL_HANDLE;
    }

    ~TextureDescriptorSet() {
        BufferManager::get().releaseTextureDescriptorSet(inner);
    }
};

}  // namespace render
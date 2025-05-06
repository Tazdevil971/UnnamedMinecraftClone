#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Managed.hpp"

namespace render {

struct Ubo;
struct BaseMesh;
struct Image;
struct Texture;

class BufferManager {
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

    ~BufferManager();

    void flushDeferOperations();

    // Mesh stuff
    template <typename T>
    T allocateMesh(const std::vector<uint16_t>& indices,
                   const std::vector<typename T::Vertex>& vertices);

    void deallocateMeshDefer(BaseMesh& mesh);
    void deallocateMeshNow(BaseMesh& mesh);

    // UBO stuff
    VkDescriptorSetLayout getUboLayout() const { return *uboLayout; }

    Ubo allocateUbo(size_t size);

    void deallocateUboDefer(Ubo& ubo);
    void deallocateUboNow(Ubo& ubo);

    // Image stuff
    Image allocateDepthImage(uint32_t width, uint32_t height);

    Image importImage(VkImage image, uint32_t width, uint32_t height,
                      VkFormat format);
    Image allocateImage(const uint8_t* pixels, uint32_t width, uint32_t height,
                        VkFormat format);
    Image allocateImage(const std::string& path, VkFormat format);

    void deallocateImageDefer(Image& image);
    void deallocateImageNow(Image& image);

    // Texture stuff
    VkDescriptorSetLayout getTextureLayout() const { return *textureLayout; }

    Texture allocateTexture(const std::string& path, VkFormat format);
    Texture allocateDepthTexture(uint32_t width, uint32_t height);

    void deallocateTextureDefer(Texture& texture);
    void deallocateTextureNow(Texture& texture);

private:
    BufferManager(size_t uboPoolSize, size_t texturePoolSize);

    BaseMesh allocateMeshInner(const void* indicesData, size_t indicesDataSize,
                               size_t indicesCount, const void* vertexData,
                               size_t vertexDataSize, size_t vertexCount);

    void cleanup();

    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();

    void createUboLayout();
    void createUboDescriptorPool(uint32_t size);
    void createUboDescriptorSets(uint32_t size);

    void createTextureLayout();
    void createTextureDescriptorPool(uint32_t size);
    void createTextureDescriptorSets(uint32_t size);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VmaMemoryUsage vmaUsage,
                      VmaAllocationCreateFlags vmaFlags, VkBuffer& outBuffer,
                      VmaAllocation& outMemory);

    void createImage(uint32_t width, uint32_t height, VkFormat format,
                     VkImageUsageFlags usage, VmaMemoryUsage vmaUsage,
                     VmaAllocationCreateFlags vmaFlags, VkImage& outImage,
                     VmaAllocation& outMemory);

    void createImageView(VkImage image, VkFormat format,
                         VkImageAspectFlags aspect, VkImageView& imageView);

    void startRecording();
    void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
    void copyBufferToImage(VkBuffer src, VkImage dst, uint32_t width,
                           uint32_t height);
    void transitionImageLayout(VkImage image, VkImageLayout oldLayout,
                               VkImageLayout newLayout, VkFormat format);
    void submitAndWait();

    std::vector<BaseMesh> meshDeallocateDefer;
    std::vector<Ubo> uboDeallocateDefer;
    std::vector<Image> imageDeallocateDefer;
    std::vector<Texture> textureDeallocateDefer;

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

}  // namespace render
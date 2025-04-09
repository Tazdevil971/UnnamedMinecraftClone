#pragma once

#include <vk_mem_alloc.h>

#include <memory>
#include <string>

#include "Context.hpp"
#include "Primitives.hpp"

namespace render {

class BufferManager {
   private:
    static std::unique_ptr<BufferManager> INSTANCE;

   public:
    static void create(uint32_t texturePoolSize) {
        INSTANCE.reset(new BufferManager(texturePoolSize));
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
    // TODO:

    // Image stuff
    DepthImage allocateDepthImage(uint32_t width, uint32_t height);

    Image importImage(VkImage image, uint32_t width,
                                  uint32_t height, VkFormat format);
    Image allocateImage(const uint8_t* pixels, uint32_t width,
                                    uint32_t height, VkFormat format);
    Image allocateImage(const std::string& path, VkFormat format);

    void deallocateImageDefer(Image& image);
    void deallocateImageNow(Image& image);

    // Texture stuff
    VkDescriptorSetLayout getTextureLayout() const { return textureLayout; }

    Texture createTexture(const std::string& path, VkFormat format);

    void deallocateTextureDefer(Texture& texture);
    void deallocateTextureNow(Texture& texture);

   private:
    BufferManager(uint32_t texturePoolSize);

    BaseMesh allocateMeshInner(const void* indicesData, size_t indicesDataSize,
                               size_t indicesCount, const void* vertexData,
                               size_t vertexDataSize, size_t vertexCount);

    void cleanup();

    void createVma();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();

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

    VmaAllocator vma{VK_NULL_HANDLE};

    std::vector<BaseMesh> meshDeallocateDefer;
    std::vector<Image> imageDeallocateDefer;
    std::vector<Texture> textureDeallocateDefer;

    std::vector<VkDescriptorSet> textureDescriptorSets;
    VkDescriptorSetLayout textureLayout{VK_NULL_HANDLE};
    VkDescriptorPool textureDescriptorPool{VK_NULL_HANDLE};

    VkCommandPool commandPool{VK_NULL_HANDLE};
    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkFence syncFence{VK_NULL_HANDLE};
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
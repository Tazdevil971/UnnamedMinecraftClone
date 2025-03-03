#pragma once

#include <vk_mem_alloc.h>

#include <memory>
#include <string>

#include "Context.hpp"
#include "Mesh.hpp"

namespace render {

class BufferManager {
   private:
    static std::unique_ptr<BufferManager> INSTANCE;

   public:
    static void create() { INSTANCE.reset(new BufferManager()); }

    static BufferManager& get() {
        if (INSTANCE) {
            return *INSTANCE;
        } else {
            throw std::runtime_error{"BufferManager not yet created"};
        }
    }

    static void destroy() { INSTANCE.reset(); }
    
    ~BufferManager();

    SimpleMesh allocateSimpleMesh(const std::vector<uint16_t>& indices,
                                  const std::vector<Vertex>& vertices);

    void deallocateSimpleMesh(SimpleMesh mesh);

    DepthImage allocateDepthImage(uint32_t width, uint32_t height);

    SimpleImage importSimpleImage(VkImage image, uint32_t width,
                                  uint32_t height, VkFormat format);
    SimpleImage allocateSimpleImage(const uint8_t* pixels, uint32_t width,
                                    uint32_t height, VkFormat format);
    SimpleImage allocateSimpleImage(const std::string& path, VkFormat format);

    void deallocateSimpleImage(SimpleImage image);

   private:
    BufferManager();

    void cleanup();

    void createVma();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();

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

    VkCommandPool commandPool{VK_NULL_HANDLE};
    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkFence syncFence{VK_NULL_HANDLE};
};

}  // namespace render
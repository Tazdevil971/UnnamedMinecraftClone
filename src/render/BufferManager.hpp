#pragma once

#include <vk_mem_alloc.h>

#include <memory>

#include "Context.hpp"
#include "Mesh.hpp"

namespace render {

class BufferManager {
   public:
    static std::shared_ptr<BufferManager> create(std::shared_ptr<Context> ctx) {
        return std::make_shared<BufferManager>(std::move(ctx));
    }

    BufferManager(std::shared_ptr<Context> ctx);

    void cleanup();

    SimpleMesh allocateSimpleMesh(const std::vector<uint16_t>& indices,
                                  const std::vector<Vertex>& vertices);

    void deallocateSimpleMesh(SimpleMesh mesh);

   private:
    void createVma();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VmaMemoryUsage vmaUsage,
                      VmaAllocationCreateFlags vmaFlags, VkBuffer& outBuffer,
                      VmaAllocation& outMemory);

    void startRecording();
    void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
    void transferBufferToGraphics(VkBuffer buffer);
    void submitAndWait();

    std::shared_ptr<Context> ctx;

    VmaAllocator vma{VK_NULL_HANDLE};

    VkCommandPool commandPool{VK_NULL_HANDLE};
    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkFence syncFence{VK_NULL_HANDLE};
};

}  // namespace render
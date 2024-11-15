#include "BufferManager.hpp"

#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace render;

BufferManager::BufferManager(std::shared_ptr<Context> ctx)
    : ctx{std::move(ctx)} {
    try {
        createVma();
        createCommandPool();
        createCommandBuffer();
        createSyncObjects();
    } catch (...) {
        cleanup();
        throw;
    }
}

void BufferManager::cleanup() {
    if (syncFence) vkDestroyFence(ctx->getDevice(), syncFence, nullptr);

    if (commandPool)
        vkDestroyCommandPool(ctx->getDevice(), commandPool, nullptr);

    if (vma != VK_NULL_HANDLE) vmaDestroyAllocator(vma);
}

SimpleMesh BufferManager::allocateSimpleMesh(
    const std::vector<uint16_t>& indices, const std::vector<Vertex>& vertices) {
    VkDeviceSize bufferSize =
        sizeof(Vertex) * vertices.size() + sizeof(uint16_t) * indices.size();

    VkDeviceSize vertexOffset = 0;
    VkDeviceSize indicesOffset = sizeof(Vertex) * vertices.size();

    VmaAllocation stagingMemory;
    VkBuffer stagingBuffer;

    VmaAllocation memory;
    VkBuffer buffer;

    // First create and fill up staging buffer
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_AUTO,
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                 stagingBuffer, stagingMemory);

    void* data;
    vmaMapMemory(vma, stagingMemory, &data);

    std::memcpy(reinterpret_cast<uint8_t*>(data) + vertexOffset,
                vertices.data(), vertices.size() * sizeof(Vertex));
    std::memcpy(reinterpret_cast<uint8_t*>(data) + indicesOffset,
                indices.data(), indices.size() * sizeof(uint16_t));

    vmaUnmapMemory(vma, stagingMemory);

    // Then create actual buffer
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VMA_MEMORY_USAGE_AUTO, 0, buffer, memory);

    // Finally copy the buffer and transfer to graphics queue
    startRecording();
    copyBuffer(stagingBuffer, buffer, bufferSize);
    transferBufferToGraphics(buffer);
    submitAndWait();

    vmaDestroyBuffer(vma, stagingBuffer, stagingMemory);

    return {memory,        buffer,          vertexOffset,
            indicesOffset, vertices.size(), indices.size()};
}

void BufferManager::deallocateSimpleMesh(SimpleMesh mesh) {
    vmaDestroyBuffer(vma, mesh.buffer, mesh.memory);
}

void BufferManager::createVma() {
    VmaAllocatorCreateInfo createInfo{};
    createInfo.flags = 0;
    createInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    createInfo.instance = ctx->getInstance();
    createInfo.physicalDevice = ctx->getDeviceInfo().device;
    createInfo.device = ctx->getDevice();

    if (ctx->getDeviceInfo().hasKHRDedicatedAllocation)
        createInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

    if (vmaCreateAllocator(&createInfo, &vma) != VK_SUCCESS)
        throw std::runtime_error{"failed to create VMA!"};
}

void BufferManager::createCommandPool() {
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
                       VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = ctx->getDeviceInfo().queues.transfer.value();

    if (vkCreateCommandPool(ctx->getDevice(), &createInfo, nullptr,
                            &commandPool) != VK_SUCCESS)
        throw std::runtime_error{"failed to create command pool!"};
}

void BufferManager::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(ctx->getDevice(), &allocInfo,
                                 &commandBuffer) != VK_SUCCESS)
        throw std::runtime_error{"failed to create command buffer!"};
}

void BufferManager::createSyncObjects() {
    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (vkCreateFence(ctx->getDevice(), &createInfo, nullptr, &syncFence) !=
        VK_SUCCESS)
        throw std::runtime_error{"failed to create sync fence!"};
}

void BufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                 VmaMemoryUsage vmaUsage,
                                 VmaAllocationCreateFlags vmaFlags,
                                 VkBuffer& outBuffer,
                                 VmaAllocation& outMemory) {
    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = vmaFlags;
    allocInfo.usage = vmaUsage;

    if (vmaCreateBuffer(vma, &createInfo, &allocInfo, &outBuffer, &outMemory,
                        nullptr) != VK_SUCCESS)
        throw std::runtime_error{"failed to create vma buffer!"};
}

void BufferManager::startRecording() {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void BufferManager::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);
}

void BufferManager::transferBufferToGraphics(VkBuffer buffer) {
    // If the device has no dedicated transfer queue this is not needed
    if (!ctx->getDeviceInfo().queues.hasDedicatedTransferQueue()) return;

    VkBufferMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.srcQueueFamilyIndex = ctx->getDeviceInfo().queues.transfer.value();
    barrier.dstQueueFamilyIndex = ctx->getDeviceInfo().queues.graphics.value();
    barrier.buffer = buffer;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 1,
                         &barrier, 0, nullptr);
}

void BufferManager::submitAndWait() {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(ctx->getTransferQueue(), 1, &submitInfo, syncFence);
    vkWaitForFences(ctx->getDevice(), 1, &syncFence, VK_TRUE, UINT64_MAX);

    vkResetCommandBuffer(commandBuffer, 0);
    vkResetFences(ctx->getDevice(), 1, &syncFence);
}
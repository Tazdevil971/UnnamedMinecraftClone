#include "BufferManager.hpp"

#include <stb_image.h>

#include <cstring>
#include <iostream>
#include <stdexcept>

#include "../Utils.hpp"

using namespace render;

std::unique_ptr<BufferManager> BufferManager::INSTANCE;

BufferManager::BufferManager() {
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

BufferManager::~BufferManager() { cleanup(); }

void BufferManager::cleanup() {
    flushDeferOperations();

    if (syncFence != VK_NULL_HANDLE) {
        vkDestroyFence(Context::get().getDevice(), syncFence, nullptr);
        syncFence = VK_NULL_HANDLE;
    }

    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(Context::get().getDevice(), commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }

    if (vma != VK_NULL_HANDLE) {
        vmaDestroyAllocator(vma);
        vma = VK_NULL_HANDLE;
    }
}

void BufferManager::flushDeferOperations() {
    for (auto& simpleImage : simpleImageDeallocateDefer)
        deallocateSimpleImageNow(simpleImage);

    for (auto& simpleMesh : simpleMeshDeallocateDefer)
        deallocateSimpleMeshNow(simpleMesh);

    simpleImageDeallocateDefer.clear();
    simpleMeshDeallocateDefer.clear();
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

    auto stagingDefer =
        Defer{[=]() { vmaDestroyBuffer(vma, stagingBuffer, stagingMemory); }};

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

    auto outputDefer = Defer{[=]() { vmaDestroyBuffer(vma, buffer, memory); }};

    // Finally copy the buffer and transfer to graphics queue
    startRecording();
    copyBuffer(stagingBuffer, buffer, bufferSize);
    submitAndWait();

    outputDefer.defuse();
    return {memory,        buffer,          vertexOffset,
            indicesOffset, vertices.size(), indices.size()};
}

void BufferManager::deallocateSimpleMeshDefer(SimpleMesh& mesh) {
    simpleMeshDeallocateDefer.push_back(mesh);
    mesh = SimpleMesh{};
}

void BufferManager::deallocateSimpleMeshNow(SimpleMesh& mesh) {
    if (mesh.memory != VK_NULL_HANDLE)
        vmaDestroyBuffer(vma, mesh.buffer, mesh.memory);

    mesh = SimpleMesh{};
}

DepthImage BufferManager::allocateDepthImage(uint32_t width, uint32_t height) {
    VmaAllocation memory;
    VkImage image;
    VkFormat format = Context::get().getDeviceInfo().depthFormat;

    createImage(width, height, format,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VMA_MEMORY_USAGE_AUTO, 0, image, memory);

    auto outputDefer = Defer{[=]() { vmaDestroyImage(vma, image, memory); }};

    startRecording();
    transitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                          format);
    submitAndWait();

    VkImageView imageView;
    createImageView(image, format, VK_IMAGE_ASPECT_DEPTH_BIT, imageView);

    outputDefer.defuse();
    return {memory, image, imageView, width, height, format};
}

SimpleImage BufferManager::importSimpleImage(VkImage image, uint32_t width,
                                             uint32_t height, VkFormat format) {
    VkImageView imageView;
    createImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, imageView);

    return {VK_NULL_HANDLE, image, imageView, width, height, format};
}

SimpleImage BufferManager::allocateSimpleImage(const std::string& path,
                                               VkFormat format) {
    int width, height, channels;
    stbi_uc* pixels =
        stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) throw std::runtime_error{"failed to laod texture file!"};

    auto pixelsDefer = Defer{[=]() { stbi_image_free(pixels); }};
    return allocateSimpleImage(pixels, width, height, format);
}

SimpleImage BufferManager::allocateSimpleImage(const uint8_t* pixels,
                                               uint32_t height, uint32_t width,
                                               VkFormat format) {
    VkDeviceSize imageSize = width * height * 4;
    VmaAllocation stagingMemory;
    VkBuffer stagingBuffer;
    VmaAllocation memory;
    VkImage image;

    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_AUTO,
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                 stagingBuffer, stagingMemory);

    auto stagingDefer =
        Defer{[=]() { vmaDestroyBuffer(vma, stagingBuffer, stagingMemory); }};

    void* data;
    vmaMapMemory(vma, stagingMemory, &data);
    memcpy(data, pixels, imageSize);
    vmaUnmapMemory(vma, stagingMemory);

    createImage(width, height, format,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VMA_MEMORY_USAGE_AUTO, 0, image, memory);

    auto outputDefer = Defer{[=]() { vmaDestroyImage(vma, image, memory); }};

    startRecording();
    transitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, format);
    copyBufferToImage(stagingBuffer, image, width, height);
    transitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, format);
    submitAndWait();

    VkImageView imageView;
    createImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, imageView);

    outputDefer.defuse();
    return {memory, image, imageView, width, height, format};
}

void BufferManager::deallocateSimpleImageDefer(SimpleImage& image) {
    simpleImageDeallocateDefer.push_back(image);
    image = SimpleImage{};
}

void BufferManager::deallocateSimpleImageNow(SimpleImage& image) {
    if (image.view != VK_NULL_HANDLE)
        vkDestroyImageView(Context::get().getDevice(), image.view, nullptr);

    if (image.memory != VK_NULL_HANDLE)
        vmaDestroyImage(vma, image.image, image.memory);

    image = SimpleImage{};
}

void BufferManager::createVma() {
    VmaAllocatorCreateInfo createInfo{};
    createInfo.flags = 0;
    createInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    createInfo.instance = Context::get().getInstance();
    createInfo.physicalDevice = Context::get().getDeviceInfo().device;
    createInfo.device = Context::get().getDevice();

    if (Context::get().getDeviceInfo().hasKHRDedicatedAllocation)
        createInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

    if (vmaCreateAllocator(&createInfo, &vma) != VK_SUCCESS)
        throw std::runtime_error{"failed to create VMA!"};
}

void BufferManager::createCommandPool() {
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
                       VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex =
        Context::get().getDeviceInfo().queues.graphics.value();

    if (vkCreateCommandPool(Context::get().getDevice(), &createInfo, nullptr,
                            &commandPool) != VK_SUCCESS)
        throw std::runtime_error{"failed to create command pool!"};
}

void BufferManager::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(Context::get().getDevice(), &allocInfo,
                                 &commandBuffer) != VK_SUCCESS)
        throw std::runtime_error{"failed to create command buffer!"};
}

void BufferManager::createSyncObjects() {
    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (vkCreateFence(Context::get().getDevice(), &createInfo, nullptr,
                      &syncFence) != VK_SUCCESS)
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

void BufferManager::createImage(uint32_t width, uint32_t height,
                                VkFormat format, VkImageUsageFlags usage,
                                VmaMemoryUsage vmaUsage,
                                VmaAllocationCreateFlags vmaFlags,
                                VkImage& outImage, VmaAllocation& outMemory) {
    VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.extent.width = width;
    createInfo.extent.height = height;
    createInfo.extent.depth = 1;
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.format = format;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.usage = usage;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = vmaFlags;
    allocInfo.usage = vmaUsage;

    if (vmaCreateImage(vma, &createInfo, &allocInfo, &outImage, &outMemory,
                       nullptr) != VK_SUCCESS)
        throw std::runtime_error{"failed to create vma image!"};
}

void BufferManager::createImageView(VkImage image, VkFormat format,
                                    VkImageAspectFlags aspect,
                                    VkImageView& imageView) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.subresourceRange.aspectMask = aspect;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(Context::get().getDevice(), &createInfo, nullptr,
                          &imageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view!");
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

void BufferManager::copyBufferToImage(VkBuffer src, VkImage dst, uint32_t width,
                                      uint32_t height) {
    VkBufferImageCopy copyRegion{};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;

    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;

    copyRegion.imageOffset = {0, 0, 0};
    copyRegion.imageExtent = {width, height, 1};
    vkCmdCopyBufferToImage(commandBuffer, src, dst,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &copyRegion);
}

void BufferManager::transitionImageLayout(VkImage image,
                                          VkImageLayout oldLayout,
                                          VkImageLayout newLayout,
                                          VkFormat format) {
    VkAccessFlags srcAccessMask;
    VkPipelineStageFlags srcStage;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        srcAccessMask = 0;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    VkAccessFlags dstAccessMask;
    VkPipelineStageFlags dstStage;
    if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    VkImageAspectFlags aspectMask;
    if (format == VK_FORMAT_D32_SFLOAT) {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
               format == VK_FORMAT_D24_UNORM_S8_UINT) {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);
}

void BufferManager::submitAndWait() {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(Context::get().getGraphicsQueue(), 1, &submitInfo, syncFence);
    vkWaitForFences(Context::get().getDevice(), 1, &syncFence, VK_TRUE,
                    UINT64_MAX);

    vkResetCommandBuffer(commandBuffer, 0);
    vkResetFences(Context::get().getDevice(), 1, &syncFence);
}
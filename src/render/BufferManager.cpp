#include "BufferManager.hpp"

#include <stb_image.h>
#include <vulkan/vulkan_core.h>

#include <cstring>
#include <stdexcept>

#include "Context.hpp"
#include "Managed.hpp"
#include "Primitives.hpp"

using namespace render;

std::unique_ptr<BufferManager> BufferManager::INSTANCE;

BufferManager::BufferManager(size_t uboPoolSize, size_t texturePoolSize) {
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();

    // Create ubo stuff
    createUboLayout();
    createUboDescriptorPool(uboPoolSize);
    createUboDescriptorSets(uboPoolSize);

    // Create texture stuff
    createTextureLayout();
    createTextureDescriptorPool(texturePoolSize);
    createTextureDescriptorSets(texturePoolSize);
}

void BufferManager::performDeferOps() { meshDefer.clear(); }

BaseMesh BufferManager::allocateMeshInner(
    const void* indicesData, size_t indicesDataSize, size_t indicesCount,
    const void* vertexData, size_t vertexDataSize, size_t vertexCount) {
    VkDeviceSize bufferSize = vertexDataSize + indicesDataSize;

    VkDeviceSize vertexOffset = 0;
    VkDeviceSize indicesOffset = vertexDataSize;

    // First create and fill up staging buffer
    ManagedBuffer stagingBuffer = createBuffer(
        bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* data;
    vmaMapMemory(Context::get().getVma(), stagingBuffer.getMemory(), &data);

    std::memcpy(reinterpret_cast<uint8_t*>(data) + vertexOffset, vertexData,
                vertexDataSize);
    std::memcpy(reinterpret_cast<uint8_t*>(data) + indicesOffset, indicesData,
                indicesDataSize);

    vmaUnmapMemory(Context::get().getVma(), stagingBuffer.getMemory());

    // Then create actual buffer
    ManagedBuffer buffer = createBuffer(bufferSize,
                                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                        VMA_MEMORY_USAGE_AUTO, 0);

    // Finally copy the buffer and transfer to graphics queue
    startRecording();
    copyBuffer(*stagingBuffer, *buffer, bufferSize);
    submitAndWait();

    return {std::move(buffer), vertexOffset, indicesOffset, vertexCount,
            indicesCount};
}

void BufferManager::deallocateMeshDefer(BaseMesh&& mesh) {
    meshDefer.push_back(std::move(mesh));
}

Ubo BufferManager::allocateUbo(size_t size) {
    ManagedBuffer buffer = createBuffer(
        size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* ptr;
    vmaMapMemory(Context::get().getVma(), buffer.getMemory(), &ptr);

    VkDescriptorSet descriptor = uboDescriptorSets.back();
    uboDescriptorSets.pop_back();

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = *buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = size;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptor;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(Context::get().getDevice(), 1, &descriptorWrite, 0,
                           nullptr);

    return {std::move(buffer), descriptor, ptr, size};
}

Image BufferManager::allocateDepthImage(uint32_t width, uint32_t height) {
    VkFormat format = Context::get().getDeviceInfo().depthFormat;

    ManagedImage image =
        createImage(width, height, format,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                        VK_IMAGE_USAGE_SAMPLED_BIT,
                    VMA_MEMORY_USAGE_AUTO, 0);

    startRecording();
    transitionImageLayout(*image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                          format);
    submitAndWait();

    ManagedImageView imageView =
        createImageView(*image, format, VK_IMAGE_ASPECT_DEPTH_BIT);

    return {std::move(image), std::move(imageView), width, height, format};
}

Image BufferManager::importImage(VkImage image, uint32_t width, uint32_t height,
                                 VkFormat format) {
    ManagedImageView imageView =
        createImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT);

    return {image, std::move(imageView), width, height, format};
}

Image BufferManager::allocateImage(const std::string& path, VkFormat format) {
    int width, height, channels;
    stbi_uc* pixels =
        stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) throw std::runtime_error{"failed to laod texture file!"};

    try {
        return allocateImage(pixels, width, height, format);
    } catch (...) {
        stbi_image_free(pixels);
        throw;
    }
}

Image BufferManager::allocateImage(const uint8_t* pixels, uint32_t height,
                                   uint32_t width, VkFormat format) {
    VkDeviceSize imageSize = width * height * 4;

    ManagedBuffer stagingBuffer = createBuffer(
        imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* data;
    vmaMapMemory(Context::get().getVma(), stagingBuffer.getMemory(), &data);
    memcpy(data, pixels, imageSize);
    vmaUnmapMemory(Context::get().getVma(), stagingBuffer.getMemory());

    ManagedImage image = createImage(
        width, height, format,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_AUTO, 0);

    startRecording();
    transitionImageLayout(*image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, format);
    copyBufferToImage(*stagingBuffer, *image, width, height);
    transitionImageLayout(*image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, format);
    submitAndWait();

    ManagedImageView imageView =
        createImageView(*image, format, VK_IMAGE_ASPECT_COLOR_BIT);

    return {std::move(image), std::move(imageView), width, height, format};
}

Texture BufferManager::allocateTexture(const std::string& path,
                                       VkFormat format) {
    Image image = allocateImage(path, format);

    VkSamplerCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = VK_FILTER_NEAREST;
    createInfo.minFilter = VK_FILTER_NEAREST;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.anisotropyEnable = VK_TRUE;
    createInfo.maxAnisotropy =
        Context::get().getDeviceInfo().maxSamplerAnisotropy;
    createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.mipLodBias = 0.0f;
    createInfo.minLod = 0.0f;
    createInfo.maxLod = 0.0f;

    ManagedSampler sampler;

    if (vkCreateSampler(Context::get().getDevice(), &createInfo, nullptr,
                        &*sampler) != VK_SUCCESS)
        throw std::runtime_error{"failed to create texture sampler!"};

    if (textureDescriptorSets.size() == 0)
        throw std::runtime_error{"not enough descriptor sets!"};

    VkDescriptorSet descriptor = textureDescriptorSets.back();
    textureDescriptorSets.pop_back();

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = *image.view;
    imageInfo.sampler = *sampler;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptor;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = nullptr;
    descriptorWrite.pImageInfo = &imageInfo;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(Context::get().getDevice(), 1, &descriptorWrite, 0,
                           nullptr);

    return {std::move(image), std::move(sampler), descriptor};
}

Texture BufferManager::allocateDepthTexture(uint32_t width, uint32_t height) {
    Image image = allocateDepthImage(width, height);

    VkSamplerCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = VK_FILTER_LINEAR;
    createInfo.minFilter = VK_FILTER_LINEAR;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.anisotropyEnable = VK_TRUE;
    createInfo.maxAnisotropy =
        Context::get().getDeviceInfo().maxSamplerAnisotropy;
    createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.mipLodBias = 0.0f;
    createInfo.minLod = 0.0f;
    createInfo.maxLod = 0.0f;

    ManagedSampler sampler;

    if (vkCreateSampler(Context::get().getDevice(), &createInfo, nullptr,
                        &*sampler) != VK_SUCCESS)
        throw std::runtime_error{"failed to create texture sampler!"};

    if (textureDescriptorSets.size() == 0)
        throw std::runtime_error{"not enough descriptor sets!"};

    VkDescriptorSet descriptor = textureDescriptorSets.back();
    textureDescriptorSets.pop_back();

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    imageInfo.imageView = *image.view;
    imageInfo.sampler = *sampler;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptor;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = nullptr;
    descriptorWrite.pImageInfo = &imageInfo;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(Context::get().getDevice(), 1, &descriptorWrite, 0,
                           nullptr);

    return {std::move(image), std::move(sampler), descriptor};
}

void BufferManager::createCommandPool() {
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
                       VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex =
        Context::get().getDeviceInfo().queues.graphics.value();

    if (vkCreateCommandPool(Context::get().getDevice(), &createInfo, nullptr,
                            &*commandPool) != VK_SUCCESS)
        throw std::runtime_error{"failed to create command pool!"};
}

void BufferManager::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = *commandPool;
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
                      &*syncFence) != VK_SUCCESS)
        throw std::runtime_error{"failed to create sync fence!"};
}

void BufferManager::createUboLayout() {
    VkDescriptorSetLayoutBinding uniformLayoutBinding{};
    uniformLayoutBinding.binding = 0;
    uniformLayoutBinding.descriptorCount = 1;
    uniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    uniformLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = 1;
    descriptorSetLayoutInfo.pBindings = &uniformLayoutBinding;

    if (vkCreateDescriptorSetLayout(Context::get().getDevice(),
                                    &descriptorSetLayoutInfo, nullptr,
                                    &*uboLayout) != VK_SUCCESS)
        throw std::runtime_error{
            "failed to create descriptor set layout info!"};
}

void BufferManager::createUboDescriptorPool(uint32_t size) {
    VkDescriptorPoolSize uniformPoolSize{};
    uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformPoolSize.descriptorCount = size;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &uniformPoolSize;
    poolInfo.maxSets = size;

    if (vkCreateDescriptorPool(Context::get().getDevice(), &poolInfo, nullptr,
                               &*uboDescriptorPool) != VK_SUCCESS)
        throw std::runtime_error{"failed to create descriptor pool!"};
}

void BufferManager::createUboDescriptorSets(uint32_t size) {
    std::vector<VkDescriptorSetLayout> layouts;
    layouts.resize(size, *uboLayout);

    uboDescriptorSets.resize(size);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = *uboDescriptorPool;
    allocInfo.descriptorSetCount = size;
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(Context::get().getDevice(), &allocInfo,
                                 uboDescriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error{"failed to create descriptor set!"};
}

void BufferManager::createTextureLayout() {
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = 1;
    descriptorSetLayoutInfo.pBindings = &samplerLayoutBinding;

    if (vkCreateDescriptorSetLayout(Context::get().getDevice(),
                                    &descriptorSetLayoutInfo, nullptr,
                                    &*textureLayout) != VK_SUCCESS)
        throw std::runtime_error{
            "failed to create descriptor set layout info!"};
}

void BufferManager::createTextureDescriptorPool(uint32_t size) {
    VkDescriptorPoolSize combinedSamplerPoolSize{};
    combinedSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    combinedSamplerPoolSize.descriptorCount = size;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &combinedSamplerPoolSize;
    poolInfo.maxSets = size;

    if (vkCreateDescriptorPool(Context::get().getDevice(), &poolInfo, nullptr,
                               &*textureDescriptorPool) != VK_SUCCESS)
        throw std::runtime_error{"failed to create descriptor pool!"};
}

void BufferManager::createTextureDescriptorSets(uint32_t size) {
    std::vector<VkDescriptorSetLayout> layouts;
    layouts.resize(size, *textureLayout);

    textureDescriptorSets.resize(size);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = *textureDescriptorPool;
    allocInfo.descriptorSetCount = size;
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(Context::get().getDevice(), &allocInfo,
                                 textureDescriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error{"failed to create descriptor set!"};
}

ManagedBuffer BufferManager::createBuffer(VkDeviceSize size,
                                          VkBufferUsageFlags usage,
                                          VmaMemoryUsage vmaUsage,
                                          VmaAllocationCreateFlags vmaFlags) {
    ManagedBuffer buffer;

    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = vmaFlags;
    allocInfo.usage = vmaUsage;

    if (vmaCreateBuffer(Context::get().getVma(), &createInfo, &allocInfo,
                        &*buffer, &buffer.getMemory(), nullptr) != VK_SUCCESS)
        throw std::runtime_error{
            "failed to create Context::get().getVma() buffer!"};

    return buffer;
}

ManagedImage BufferManager::createImage(uint32_t width, uint32_t height,
                                        VkFormat format,
                                        VkImageUsageFlags usage,
                                        VmaMemoryUsage vmaUsage,
                                        VmaAllocationCreateFlags vmaFlags) {
    ManagedImage image;

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

    if (vmaCreateImage(Context::get().getVma(), &createInfo, &allocInfo,
                       &*image, &image.getMemory(), nullptr) != VK_SUCCESS)
        throw std::runtime_error{
            "failed to create Context::get().getVma() image!"};

    return image;
}

ManagedImageView BufferManager::createImageView(VkImage image, VkFormat format,
                                                VkImageAspectFlags aspect) {
    ManagedImageView imageView;

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
                          &*imageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view!");

    return imageView;
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

    vkQueueSubmit(Context::get().getGraphicsQueue(), 1, &submitInfo,
                  *syncFence);
    vkWaitForFences(Context::get().getDevice(), 1, &*syncFence, VK_TRUE,
                    UINT64_MAX);

    vkResetCommandBuffer(commandBuffer, 0);
    vkResetFences(Context::get().getDevice(), 1, &*syncFence);
}
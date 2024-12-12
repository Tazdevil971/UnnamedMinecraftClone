#include "TextureManager.hpp"

#include "../Utils.hpp"

#include <iostream>
#include <stdexcept>

using namespace render;

TextureManager::TextureManager(std::shared_ptr<Context> ctx,
                               std::shared_ptr<BufferManager> bufferMgr,
                               uint32_t poolSize)
    : ctx{std::move(ctx)}, bufferMgr{std::move(bufferMgr)} {
    try {
        createLayout();
        createPool(poolSize);
        createDescriptorSets(poolSize);
    } catch (...) {
        cleanup();
        throw;
    }
}

void TextureManager::cleanup() {
    if (pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(ctx->getDevice(), pool, nullptr);

    if (simpleLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(ctx->getDevice(), simpleLayout, nullptr);
}

SimpleTexture TextureManager::createSimpleTexture(const std::string& path,
                                                  VkFormat format) {
    SimpleImage image = bufferMgr->allocateSimpleImage(path, format);

    auto imageDefer = Defer{[&]() {
        bufferMgr->deallocateSimpleImage(image);
    }};

    VkSamplerCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = VK_FILTER_LINEAR;
    createInfo.minFilter = VK_FILTER_LINEAR;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.anisotropyEnable = VK_TRUE;
    createInfo.maxAnisotropy = ctx->getDeviceInfo().maxSamplerAnisotropy;
    createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.mipLodBias = 0.0f;
    createInfo.minLod = 0.0f;
    createInfo.maxLod = 0.0f;

    VkSampler sampler;

    if (vkCreateSampler(ctx->getDevice(), &createInfo, nullptr,
                                  &sampler) != VK_SUCCESS)
        throw std::runtime_error{"failed to create texture sampler!"};

    auto samplerDefer = Defer{[&]() {
        vkDestroySampler(ctx->getDevice(), sampler, nullptr);
    }};


    if (descriptorSets.size() == 0)
        throw std::runtime_error{"not enough descriptor sets!"};

    VkDescriptorSet descriptor = descriptorSets.back();
    descriptorSets.pop_back();

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = image.view;
    imageInfo.sampler = sampler;

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

    vkUpdateDescriptorSets(ctx->getDevice(), 1, &descriptorWrite, 0, nullptr);

    imageDefer.defuse();
    samplerDefer.defuse();
    return {image, sampler, descriptor};
}

void TextureManager::deallocateSimpleTexture(SimpleTexture texture) {
    if (texture.descriptor != VK_NULL_HANDLE)
        descriptorSets.push_back(texture.descriptor);

    if (texture.sampler != VK_NULL_HANDLE)
        vkDestroySampler(ctx->getDevice(), texture.sampler, nullptr);

    bufferMgr->deallocateSimpleImage(texture.image);
}

void TextureManager::createLayout() {
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

    if (vkCreateDescriptorSetLayout(ctx->getDevice(), &descriptorSetLayoutInfo,
                                    nullptr, &simpleLayout) != VK_SUCCESS)
        throw std::runtime_error{
            "failed to create descriptor set layout info!"};
}

void TextureManager::createPool(uint32_t size) {
    VkDescriptorPoolSize combinedSamplerPoolSize{};
    combinedSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    combinedSamplerPoolSize.descriptorCount = size;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &combinedSamplerPoolSize;
    poolInfo.maxSets = size;

    if (vkCreateDescriptorPool(ctx->getDevice(), &poolInfo, nullptr, &pool) !=
        VK_SUCCESS)
        throw std::runtime_error{"failed to create descriptor pool!"};
}

void TextureManager::createDescriptorSets(uint32_t size) {
    std::vector<VkDescriptorSetLayout> layouts;
    layouts.resize(size, simpleLayout);

    descriptorSets.resize(size);    
    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = size;
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(ctx->getDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error{"failed to create descriptor set!"};
}

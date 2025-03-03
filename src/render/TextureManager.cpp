#include "TextureManager.hpp"

#include <iostream>
#include <stdexcept>

#include "../Utils.hpp"

using namespace render;

std::unique_ptr<TextureManager> TextureManager::INSTANCE;

TextureManager::TextureManager(uint32_t poolSize) {
    try {
        createLayout();
        createPool(poolSize);
        createDescriptorSets(poolSize);
    } catch (...) {
        cleanup();
        throw;
    }
}

TextureManager::~TextureManager() { cleanup(); }

void TextureManager::cleanup() {
    flushDeferOperations();

    if (pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(Context::get().getDevice(), pool, nullptr);
        pool = VK_NULL_HANDLE;
    }

    if (simpleLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(Context::get().getDevice(), simpleLayout,
                                     nullptr);
        simpleLayout = VK_NULL_HANDLE;
    }
}

void TextureManager::flushDeferOperations() {
    for (auto& simpleTexture : simpleTextureDeallocateDefer)
        deallocateSimpleTextureNow(simpleTexture);

    simpleTextureDeallocateDefer.clear();
}

SimpleTexture TextureManager::createSimpleTexture(const std::string& path,
                                                  VkFormat format) {
    SimpleImage image = BufferManager::get().allocateSimpleImage(path, format);

    auto imageDefer =
        Defer{[&]() { BufferManager::get().deallocateSimpleImageNow(image); }};

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

    VkSampler sampler;

    if (vkCreateSampler(Context::get().getDevice(), &createInfo, nullptr,
                        &sampler) != VK_SUCCESS)
        throw std::runtime_error{"failed to create texture sampler!"};

    auto samplerDefer = Defer{[&]() {
        vkDestroySampler(Context::get().getDevice(), sampler, nullptr);
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

    vkUpdateDescriptorSets(Context::get().getDevice(), 1, &descriptorWrite, 0,
                           nullptr);

    imageDefer.defuse();
    samplerDefer.defuse();
    return {image, sampler, descriptor};
}

void TextureManager::deallocateSimpleTextureDefer(SimpleTexture& texture) {
    simpleTextureDeallocateDefer.push_back(texture);
    texture = SimpleTexture{};
}

void TextureManager::deallocateSimpleTextureNow(SimpleTexture& texture) {
    if (texture.descriptor != VK_NULL_HANDLE)
        descriptorSets.push_back(texture.descriptor);

    if (texture.sampler != VK_NULL_HANDLE)
        vkDestroySampler(Context::get().getDevice(), texture.sampler, nullptr);

    BufferManager::get().deallocateSimpleImageNow(texture.image);
    texture = SimpleTexture{};
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

    if (vkCreateDescriptorSetLayout(Context::get().getDevice(),
                                    &descriptorSetLayoutInfo, nullptr,
                                    &simpleLayout) != VK_SUCCESS)
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

    if (vkCreateDescriptorPool(Context::get().getDevice(), &poolInfo, nullptr,
                               &pool) != VK_SUCCESS)
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

    if (vkAllocateDescriptorSets(Context::get().getDevice(), &allocInfo,
                                 descriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error{"failed to create descriptor set!"};
}

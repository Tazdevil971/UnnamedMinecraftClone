#include "Framebuffer.hpp"

#include "BufferManager.hpp"
#include "Context.hpp"

using namespace render;

Framebuffer::Framebuffer(VkSwapchainKHR swapchain, VkExtent2D extent,
                         VkFormat colorFormat, VkRenderPass renderPass)
    : renderPass{renderPass}, extent{extent}, colorFormat{colorFormat} {
    try {
        getImageCount(swapchain);
        createColorImages(swapchain);
        createDepthImages();
        createFramebuffers();
    } catch (...) {
        cleanup();
        throw;
    }
}

void Framebuffer::recreate(VkSwapchainKHR swapchain, VkExtent2D extent,
                           VkFormat colorFormat) {
    for (auto framebuffer : framebuffers)
        if (framebuffer != VK_NULL_HANDLE)
            vkDestroyFramebuffer(Context::get().getDevice(), framebuffer,
                                 nullptr);

    for (auto colorImage : colorImages)
        BufferManager::get().deallocateImageNow(colorImage);

    for (auto depthImage : depthImages)
        BufferManager::get().deallocateImageNow(depthImage);

    framebuffers.resize(0);
    colorImages.resize(0);
    depthImages.resize(0);

    this->extent = extent;
    this->colorFormat = colorFormat;
    getImageCount(swapchain);

    createColorImages(swapchain);
    createDepthImages();
    createFramebuffers();
}

void Framebuffer::cleanup() {
    for (auto& framebuffer : framebuffers) {
        if (framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(Context::get().getDevice(), framebuffer,
                                 nullptr);
            framebuffer = VK_NULL_HANDLE;
        }
    }

    for (auto colorImage : colorImages)
        BufferManager::get().deallocateImageNow(colorImage);

    for (auto depthImage : depthImages)
        BufferManager::get().deallocateImageNow(depthImage);

    framebuffers.resize(0);
    colorImages.resize(0);
    depthImages.resize(0);
}

void Framebuffer::getImageCount(VkSwapchainKHR swapchain) {
    vkGetSwapchainImagesKHR(Context::get().getDevice(), swapchain, &imageCount,
                            nullptr);
}

void Framebuffer::createColorImages(VkSwapchainKHR swapchain) {
    std::vector<VkImage> importedImages;
    importedImages.resize(imageCount, VK_NULL_HANDLE);
    vkGetSwapchainImagesKHR(Context::get().getDevice(), swapchain, &imageCount,
                            importedImages.data());

    colorImages.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
        colorImages[i] = BufferManager::get().importImage(
            importedImages[i], extent.width, extent.height, colorFormat);
    }
}

void Framebuffer::createDepthImages() {
    depthImages.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
        depthImages[i] = BufferManager::get().allocateDepthImage(extent.width,
                                                                 extent.height);
    }
}

void Framebuffer::createFramebuffers() {
    framebuffers.resize(imageCount, VK_NULL_HANDLE);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageView attachments[] = {colorImages[i].view, depthImages[i].view};

        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = renderPass;
        createInfo.attachmentCount = 2;
        createInfo.pAttachments = attachments;
        createInfo.width = extent.width;
        createInfo.height = extent.height;
        createInfo.layers = 1;

        if (vkCreateFramebuffer(Context::get().getDevice(), &createInfo,
                                nullptr, &framebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error{"failed to create framebuffer!"};
    }
}
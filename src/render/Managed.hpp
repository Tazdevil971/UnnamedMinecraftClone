#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Context.hpp"

namespace render {

template <typename T, void (*F)(VkDevice, T, const VkAllocationCallbacks*)>
struct Managed1 {
    T inner{VK_NULL_HANDLE};

    Managed1() = default;
    Managed1(T inner) : inner{inner} {}
    Managed1(const Managed1&) = delete;
    Managed1& operator=(const Managed1&) = delete;

    Managed1& operator=(Managed1&& other) {
        inner = other.inner;
        other.inner = VK_NULL_HANDLE;
        return *this;
    }

    Managed1(Managed1&& other) {
        inner = other.inner;
        other.inner = VK_NULL_HANDLE;
    }

    ~Managed1() {
        if (inner != VK_NULL_HANDLE)
            F(Context::get().getDevice(), inner, nullptr);
        inner = VK_NULL_HANDLE;
    }

    bool isNull() const { return inner == VK_NULL_HANDLE; }

    T take() {
        T ret = inner;
        inner = VK_NULL_HANDLE;
        return ret;
    }

    T& operator*() { return inner; }
    const T& operator*() const { return inner; }
};

template <typename T, void (*F)(VmaAllocator, T, VmaAllocation)>
struct Managed2 {
    VmaAllocation memory{VK_NULL_HANDLE};
    T inner{VK_NULL_HANDLE};

    Managed2() = default;
    Managed2(T inner) : inner{inner} {}
    Managed2(const Managed2&) = delete;
    Managed2& operator=(const Managed2&) = delete;

    Managed2& operator=(Managed2&& other) {
        inner = other.inner;
        memory = other.memory;
        other.inner = VK_NULL_HANDLE;
        other.memory = VK_NULL_HANDLE;
        return *this;
    }

    Managed2(Managed2&& other) {
        inner = other.inner;
        memory = other.memory;
        other.inner = VK_NULL_HANDLE;
        other.memory = VK_NULL_HANDLE;
    }

    ~Managed2() {
        if (memory != VK_NULL_HANDLE) F(Context::get().getVma(), inner, memory);
        memory = VK_NULL_HANDLE;
        inner = VK_NULL_HANDLE;
    }

    bool isNull() const { return inner == VK_NULL_HANDLE; }

    T& operator*() { return inner; }
    const T& operator*() const { return inner; }

    VmaAllocation& getMemory() { return memory; }
    const VmaAllocation& getMemory() const { return memory; }
};

using ManagedDescriptorSetLayout =
    Managed1<VkDescriptorSetLayout, vkDestroyDescriptorSetLayout>;
using ManagedDescriptorPool =
    Managed1<VkDescriptorPool, vkDestroyDescriptorPool>;
using ManagedCommandPool = Managed1<VkCommandPool, vkDestroyCommandPool>;
using ManagedFence = Managed1<VkFence, vkDestroyFence>;
using ManagedSemaphore = Managed1<VkSemaphore, vkDestroySemaphore>;
using ManagedRenderPass = Managed1<VkRenderPass, vkDestroyRenderPass>;
using ManagedFramebuffer = Managed1<VkFramebuffer, vkDestroyFramebuffer>;
using ManagedPipelineLayout =
    Managed1<VkPipelineLayout, vkDestroyPipelineLayout>;
using ManagedPipeline = Managed1<VkPipeline, vkDestroyPipeline>;
using ManagedSwapchain = Managed1<VkSwapchainKHR, vkDestroySwapchainKHR>;
using ManagedShaderModule = Managed1<VkShaderModule, vkDestroyShaderModule>;
using ManagedSampler = Managed1<VkSampler, vkDestroySampler>;
using ManagedImageView = Managed1<VkImageView, vkDestroyImageView>;

using ManagedBuffer = Managed2<VkBuffer, vmaDestroyBuffer>;
using ManagedImage = Managed2<VkImage, vmaDestroyImage>;

}  // namespace render
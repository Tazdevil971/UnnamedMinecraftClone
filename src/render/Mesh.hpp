#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <glm/glm.hpp>

namespace render {

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;
};

struct SimpleMesh {
    VmaAllocation memory{VK_NULL_HANDLE};
    VkBuffer buffer{VK_NULL_HANDLE};

    VkDeviceSize vertexOffset;
    VkDeviceSize indicesOffset;

    size_t vertexCount;
    size_t indexCount;

    void bind(VkCommandBuffer commandBuffer) {
        VkDeviceSize offsets[] = {vertexOffset};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, buffer, indicesOffset,
                             VK_INDEX_TYPE_UINT16);
    }
};

struct SimpleImage {
    VmaAllocation memory{VK_NULL_HANDLE};
    VkImage image{VK_NULL_HANDLE};
    VkImageView view{VK_NULL_HANDLE};

    uint32_t width;
    uint32_t height;
    VkFormat format;
};

struct DepthImage : SimpleImage {};

struct SimpleTexture {
    SimpleImage image;
    VkSampler sampler;
    VkDescriptorSet descriptor;
};

struct SimpleModel {
    SimpleMesh mesh;
    SimpleTexture texture;
    glm::mat4 modelMatrix;
};

}  // namespace render
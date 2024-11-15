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
    VmaAllocation memory;
    VkBuffer buffer;

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

}  // namespace render
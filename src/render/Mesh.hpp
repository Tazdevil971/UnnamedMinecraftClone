#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

    void bind(VkCommandBuffer commandBuffer) const {
        VkDeviceSize offsets[] = {vertexOffset};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, buffer, indicesOffset,
                             VK_INDEX_TYPE_UINT16);
    }
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription description{};
        description.binding = 0;
        description.stride = sizeof(Vertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return description;
    }

    static std::array<VkVertexInputAttributeDescription, 3>
    getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> descriptions{};
        descriptions[0].binding = 0;
        descriptions[0].location = 0;
        descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        descriptions[0].offset = offsetof(Vertex, pos);

        descriptions[1].binding = 0;
        descriptions[1].location = 1;
        descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        descriptions[1].offset = offsetof(Vertex, color);

        descriptions[2].binding = 0;
        descriptions[2].location = 2;
        descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        descriptions[2].offset = offsetof(Vertex, uv);

        return descriptions;
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

struct Camera {
    float fov;
    float nearPlane;
    float farPlane;
    glm::vec3 pos;
    float yaw;
    float pitch;

    glm::mat4 computeProj(float ratio) const {
        glm::mat4 proj =
            glm::perspective(glm::radians(fov), ratio, nearPlane, farPlane);
        proj[1][1] *= -1;

        return proj;
    }

    glm::mat4 computeRot() const {
        return glm::rotate(glm::mat4(1.0f), -pitch,
                           glm::vec3(1.0f, 0.0f, 0.0f)) *
               glm::rotate(glm::mat4(1.0f), -yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 computeTrans() const {
        return glm::translate(glm::mat4(1.0f),
                              pos * glm::vec3(-1.0f, 1.0f, -1.0f));
    }

    glm::mat4 computeView() const { return computeRot() * computeTrans(); }

    glm::mat4 computeVP(float ratio) const {
        return computeProj(ratio) * computeView();
    }
};

}  // namespace render
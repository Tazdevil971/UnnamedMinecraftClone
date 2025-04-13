#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace render {

struct SkyboxVertex {
    glm::vec3 pos;
    glm::vec2 uv;
};

struct GeometryVertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct UiVertex {
    glm::vec2 pos;
    glm::vec2 uv;
};

struct BaseMesh {
    VmaAllocation memory{VK_NULL_HANDLE};
    VkBuffer buffer{VK_NULL_HANDLE};

    VkDeviceSize vertexOffset{0};
    VkDeviceSize indicesOffset{0};

    size_t vertexCount{0};
    size_t indexCount{0};

    bool isNull() const {
        return memory == VK_NULL_HANDLE && buffer == VK_NULL_HANDLE;
    }

    void bind(VkCommandBuffer commandBuffer) const {
        VkDeviceSize offsets[] = {vertexOffset};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, buffer, indicesOffset,
                             VK_INDEX_TYPE_UINT16);
    }
};

struct SkyboxMesh : BaseMesh {
    using Vertex = SkyboxVertex;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription description{};
        description.binding = 0;
        description.stride = sizeof(SkyboxVertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return description;
    }

    static std::array<VkVertexInputAttributeDescription, 2>
    getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> descriptions{};
        descriptions[0].binding = 0;
        descriptions[0].location = 0;
        descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        descriptions[0].offset = offsetof(SkyboxVertex, pos);

        descriptions[1].binding = 0;
        descriptions[1].location = 1;
        descriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        descriptions[1].offset = offsetof(SkyboxVertex, uv);

        return descriptions;
    }
};

struct GeometryMesh : BaseMesh {
    using Vertex = GeometryVertex;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription description{};
        description.binding = 0;
        description.stride = sizeof(GeometryVertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return description;
    }

    static std::array<VkVertexInputAttributeDescription, 4>
    getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> descriptions{};
        descriptions[0].binding = 0;
        descriptions[0].location = 0;
        descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        descriptions[0].offset = offsetof(GeometryVertex, pos);

        descriptions[1].binding = 0;
        descriptions[1].location = 1;
        descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        descriptions[1].offset = offsetof(GeometryVertex, color);

        descriptions[2].binding = 0;
        descriptions[2].location = 2;
        descriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        descriptions[2].offset = offsetof(GeometryVertex, normal);

        descriptions[3].binding = 0;
        descriptions[3].location = 3;
        descriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        descriptions[3].offset = offsetof(GeometryVertex, uv);

        return descriptions;
    }
};

struct UiMesh : BaseMesh {
    using Vertex = UiVertex;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription description{};
        description.binding = 0;
        description.stride = sizeof(UiVertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return description;
    }

    static std::array<VkVertexInputAttributeDescription, 2>
    getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> descriptions{};
        descriptions[0].binding = 0;
        descriptions[0].location = 0;
        descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        descriptions[0].offset = offsetof(UiVertex, pos);

        descriptions[1].binding = 0;
        descriptions[1].location = 1;
        descriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        descriptions[1].offset = offsetof(UiVertex, uv);

        return descriptions;
    }
};

struct Ubo {
    VmaAllocation memory{VK_NULL_HANDLE};
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDescriptorSet descriptor{VK_NULL_HANDLE};

    bool isNull() const {
        return memory == VK_NULL_HANDLE && buffer == VK_NULL_HANDLE;
    }

    void *ptr{nullptr};
    size_t size;

    template <typename T>
    void write(const T &value) {
        assert(sizeof(T) <= size);
        *reinterpret_cast<T *>(ptr) = value;
    }
};

struct Image {
    VmaAllocation memory{VK_NULL_HANDLE};
    VkImage image{VK_NULL_HANDLE};
    VkImageView view{VK_NULL_HANDLE};

    bool isNull() const {
        return memory == VK_NULL_HANDLE && image == VK_NULL_HANDLE &&
               view == VK_NULL_HANDLE;
    }

    uint32_t width;
    uint32_t height;
    VkFormat format;
};

struct DepthImage : Image {};

struct Texture {
    Image image;
    VkSampler sampler;
    VkDescriptorSet descriptor{VK_NULL_HANDLE};
};

struct GeometryModel {
    GeometryMesh mesh;
    Texture texture;
    glm::vec3 pos;
    glm::quat rot;

    glm::mat4 computeModelMat() const {
        return glm::translate(glm::mat4(1.0f), pos) * glm::toMat4(rot);
    }

    glm::mat4 computeNormalMat() const { return glm::toMat4(rot); }
};

struct UiModel {
    UiMesh mesh;
    Texture texture;
    glm::vec2 pos;
};

struct Camera {
    float fov;
    float nearPlane;
    float farPlane;
    glm::vec3 pos;
    glm::quat rot;

    glm::mat4 computeProjMat(float ratio) const {
        glm::mat4 proj =
            glm::perspective(glm::radians(fov), ratio, nearPlane, farPlane);
        proj[1][1] *= -1;

        return proj;
    }

    glm::mat4 computeViewMat() const {
        // TODO: Can we make this faster? Removing the matrix inversion?
        return glm::translate(glm::affineInverse(glm::toMat4(rot)), -pos);
    }

    glm::mat4 computeSkyboxViewMat() const {
        // TODO: Can we make this faster? Removing the matrix inversion?
        return glm::affineInverse(glm::toMat4(rot));
    }

    glm::mat4 computeVPMat(float ratio) const {
        return computeProjMat(ratio) * computeViewMat();
    }

    glm::mat4 computeSkyboxVPMat(float ratio) const {
        return computeProjMat(ratio) * computeSkyboxViewMat();
    }

    glm::vec3 computeViewDir() const { return rot * glm::vec3{0, 0, -1.0f}; }
};

struct Light {
    glm::vec4 pos;
    glm::vec4 color;
};

}  // namespace render
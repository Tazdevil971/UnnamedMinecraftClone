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

// #include "BufferManager.hpp"
#include "Managed.hpp"
#include "glm/ext/matrix_clip_space.hpp"

namespace render {

struct SkyboxVertex {
    glm::vec3 pos;
    glm::vec2 uv;
};

struct GeometryVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 uv;
};

struct UiVertex {
    glm::vec2 pos;
    glm::vec2 uv;
};

struct BaseMesh {
    ManagedBuffer buffer;

    VkDeviceSize vertexOffset{0};
    VkDeviceSize indicesOffset{0};

    size_t vertexCount{0};
    size_t indexCount{0};

    BaseMesh() = default;
    BaseMesh(BaseMesh &&) = default;
    BaseMesh &operator=(BaseMesh &&) = default;

    bool isNull() const { return buffer.isNull(); }

    void bind(VkCommandBuffer commandBuffer) const {
        VkDeviceSize offsets[] = {vertexOffset};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &*buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, *buffer, indicesOffset,
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
        descriptions[1].offset = offsetof(GeometryVertex, normal);

        descriptions[2].binding = 0;
        descriptions[2].location = 2;
        descriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        descriptions[2].offset = offsetof(GeometryVertex, tangent);

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
    ManagedBuffer buffer;
    VkDescriptorSet descriptor{VK_NULL_HANDLE};

    void *ptr{nullptr};
    size_t size{0};

    Ubo() = default;
    Ubo(Ubo &&) = default;
    Ubo &operator=(Ubo &&) = default;

    ~Ubo() {
        if (buffer.getMemory() != VK_NULL_HANDLE)
            vmaUnmapMemory(Context::get().getVma(), buffer.getMemory());
        ptr = nullptr;
    }

    template <typename T>
    void write(const T &value) {
        assert(sizeof(T) <= size);
        *reinterpret_cast<T *>(ptr) = value;
    }
};

struct Image {
    ManagedImage image;
    ManagedImageView view;

    uint32_t width{0};
    uint32_t height{0};
    VkFormat format{VK_FORMAT_UNDEFINED};

    Image() = default;
    Image(Image &&) = default;
    Image &operator=(Image &&) = default;
};

struct Texture {
    Image image;
    ManagedSampler sampler{VK_NULL_HANDLE};
    VkDescriptorSet descriptor{VK_NULL_HANDLE};

    Texture() = default;
    Texture(Texture &&) = default;
    Texture &operator=(Texture &&) = default;
};

struct GeometryModel {
    const GeometryMesh &mesh;
    const Texture &texture;
    glm::vec3 pos;
    glm::quat rot;

    glm::mat4 computeModelMat() const {
        return glm::translate(glm::mat4(1.0f), pos) * glm::toMat4(rot);
    }
};

struct UiModel {
    const UiMesh &mesh;
    const Texture &texture;
    glm::vec2 pos;
    glm::vec2 anchorPoint;
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

    glm::mat4 computeRotMat() const {
        // TODO: Can we make this faster? Removing the matrix inversion?
        return glm::affineInverse(glm::toMat4(rot));
    }

    glm::mat4 computeViewMat() const {
        return glm::translate(computeRotMat(), -pos);
    }

    glm::mat4 computeVPMat(float ratio) const {
        return computeProjMat(ratio) * computeViewMat();
    }

    glm::mat4 computeSkyboxVPMat(float ratio) const {
        return computeProjMat(ratio) * computeRotMat();
    }

    glm::vec3 computeViewDir() const { return rot * glm::vec3{0, 0, -1.0f}; }
};

struct Light {
    glm::vec4 pos;
    glm::vec4 color;
};

static constexpr glm::vec2 CENTER = {0, 0};
static constexpr glm::vec2 BOTTOM_CENTER = {0, 1};

}  // namespace render
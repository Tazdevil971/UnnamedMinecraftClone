#pragma once

#include "Context.hpp"
#include "BufferManager.hpp"
#include "Mesh.hpp"

namespace render {

class TextureManager {
public:
    TextureManager(std::shared_ptr<Context> ctx, std::shared_ptr<BufferManager> bufferMgr, uint32_t poolSize);

    void cleanup();

    VkDescriptorSetLayout getSimpleLayout() const { return simpleLayout; }

    SimpleTexture createSimpleTexture(const std::string& path);

    void deallocateSimpleTexture(SimpleTexture texture);

private:
    void createLayout();
    void createPool(uint32_t size);

    std::shared_ptr<Context> ctx;
    std::shared_ptr<BufferManager> bufferMgr;

    VkDescriptorSetLayout simpleLayout{VK_NULL_HANDLE};
    VkDescriptorPool pool{VK_NULL_HANDLE};
};

}
#pragma once

#include <vector>

#include "BufferManager.hpp"
#include "Context.hpp"
#include "Mesh.hpp"

namespace render {

class TextureManager {
   public:
    static std::shared_ptr<TextureManager> create(
        std::shared_ptr<Context> ctx, std::shared_ptr<BufferManager> bufferMgr,
        uint32_t poolSize) {
        return std::make_shared<TextureManager>(std::move(ctx),
                                                std::move(bufferMgr), poolSize);
    }

    TextureManager(std::shared_ptr<Context> ctx,
                   std::shared_ptr<BufferManager> bufferMgr, uint32_t poolSize);

    void cleanup();

    VkDescriptorSetLayout getSimpleLayout() const { return simpleLayout; }

    SimpleTexture createSimpleTexture(const std::string& path, VkFormat format);

    void deallocateSimpleTexture(SimpleTexture texture);

   private:
    void createLayout();
    void createPool(uint32_t size);
    void createDescriptorSets(uint32_t size);

    std::shared_ptr<Context> ctx;
    std::shared_ptr<BufferManager> bufferMgr;

    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSetLayout simpleLayout{VK_NULL_HANDLE};
    VkDescriptorPool pool{VK_NULL_HANDLE};
};

}  // namespace render
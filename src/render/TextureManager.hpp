#pragma once

#include <vector>

#include "BufferManager.hpp"
#include "Context.hpp"
#include "Mesh.hpp"

namespace render {

class TextureManager {
   private:
    static std::unique_ptr<TextureManager> INSTANCE;

   public:
    static void create(uint32_t poolSize) {
        INSTANCE.reset(new TextureManager(poolSize));
    }

    static TextureManager& get() {
        if (INSTANCE) {
            return *INSTANCE;
        } else {
            throw std::runtime_error{"TextureManager not yet created"};
        }
    }

    static void destroy() { INSTANCE.reset(); }

    ~TextureManager();

    VkDescriptorSetLayout getSimpleLayout() const { return simpleLayout; }

    SimpleTexture createSimpleTexture(const std::string& path, VkFormat format);

    void deallocateSimpleTexture(SimpleTexture texture);

   private:
    TextureManager(uint32_t poolSize);

    void cleanup();

    void createLayout();
    void createPool(uint32_t size);
    void createDescriptorSets(uint32_t size);

    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSetLayout simpleLayout{VK_NULL_HANDLE};
    VkDescriptorPool pool{VK_NULL_HANDLE};
};

}  // namespace render
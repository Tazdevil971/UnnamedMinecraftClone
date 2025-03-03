#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "../render/TextureManager.hpp"
#include "Block.hpp"

namespace world {

class AtlasManager {
   private:
    static std::unique_ptr<AtlasManager> INSTANCE;

   public:
    static void create() { INSTANCE.reset(new AtlasManager()); }

    static AtlasManager& get() {
        if (INSTANCE) {
            return *INSTANCE;
        } else {
            throw std::runtime_error{"AtlasManager not yet created"};
        }
    }

    static void destroy() { INSTANCE.reset(); }

    ~AtlasManager();

    struct AtlasBounds {
        glm::vec2 topLeft;
        glm::vec2 bottomRight;

        glm::vec2 getTopLeft() const { return topLeft; }
        glm::vec2 getBottomRight() const { return bottomRight; }
        glm::vec2 getTopRight() const { return {bottomRight.x, topLeft.y}; }
        glm::vec2 getBottomLeft() const { return {topLeft.x, bottomRight.y}; }
    };

    const render::SimpleTexture& getAtlas() const { return atlas; }

    AtlasBounds getAtlasBounds(Block block, Side side) const;

   private:
    AtlasManager();

    void cleanup();

    glm::vec2 convertIntCoords(glm::ivec2 coords) const;
    AtlasBounds computeAtlasBound(glm::ivec2 coords) const;

    render::SimpleTexture atlas;
};

}  // namespace world
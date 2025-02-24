#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "../render/TextureManager.hpp"
#include "Block.hpp"

namespace world {

class AtlasManager {
   public:
    struct AtlasBounds {
        glm::vec2 topLeft;
        glm::vec2 bottomRight;

        glm::vec2 getTopLeft() const { return topLeft; }

        glm::vec2 getBottomRight() const { return bottomRight; }

        glm::vec2 getTopRight() const { return {topLeft.y, bottomRight.x}; }

        glm::vec2 getBottomLeft() const { return {bottomRight.y, topLeft.x}; }
    };

    static std::shared_ptr<AtlasManager> create(
        std::shared_ptr<render::TextureManager> textureMgr) {
        return std::make_shared<AtlasManager>(std::move(textureMgr));
    }

    AtlasManager(std::shared_ptr<render::TextureManager> textureMgr);

    void cleanup();

    const render::SimpleTexture &getAtlas() const { return atlas; }

    AtlasBounds getAtlasBounds(Block block, Side side) const;

   private:
    glm::vec2 convertIntCoords(glm::ivec2 coords) const;

    render::SimpleTexture atlas;

    std::shared_ptr<render::TextureManager> textureMgr;
};

}  // namespace world
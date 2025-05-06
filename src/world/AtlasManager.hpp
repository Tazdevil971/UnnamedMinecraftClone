#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "../render/Primitives.hpp"
#include "Block.hpp"

namespace world {

class AtlasManager {
public:
    AtlasManager();

    static std::shared_ptr<AtlasManager> create() {
        return std::make_shared<AtlasManager>();
    }

    struct AtlasBounds {
        glm::vec2 topLeft;
        glm::vec2 bottomRight;

        glm::vec2 getTopLeft() const { return topLeft; }
        glm::vec2 getBottomRight() const { return bottomRight; }
        glm::vec2 getTopRight() const { return {bottomRight.x, topLeft.y}; }
        glm::vec2 getBottomLeft() const { return {topLeft.x, bottomRight.y}; }
    };

    const render::Texture& getAtlas() const { return atlas; }

    AtlasBounds getAtlasBounds(Block block, Side side) const;

private:
    glm::vec2 convertIntCoords(glm::ivec2 coords) const;
    AtlasBounds computeAtlasBound(glm::ivec2 coords) const;

    render::Texture atlas;
};

}  // namespace world
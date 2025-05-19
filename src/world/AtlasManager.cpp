#include "AtlasManager.hpp"

#include "../render/BufferManager.hpp"

using namespace world;
using namespace render;

AtlasManager::AtlasManager() {
    atlas = BufferManager::get().allocateTexture("assets/block_atlas.png",
                                                 VK_FORMAT_R8G8B8A8_SRGB);
}

AtlasManager::AtlasBounds AtlasManager::getAtlasBounds(Block block,
                                                       Side side) const {
    // DIRT
    if (block == Block::DIRT) {
        return computeAtlasBound({0, 0});

        // GRASS
    } else if (block == Block::GRASS) {
        if (side == Side::SIDE_Y_POS) {
            return computeAtlasBound({32, 0});
        } else if (side == Side::SIDE_Y_NEG) {
            return computeAtlasBound({0, 0});
        } else {
            return computeAtlasBound({16, 0});
        }
        // WOOD
    } else if (block == Block::WOOD_LOG) {
        if ((side == Side::SIDE_Y_NEG) || (side == Side::SIDE_Y_POS)) {
            return computeAtlasBound({32, 16});
        } else {
            return computeAtlasBound({48, 0});
        }
        // COBBLESTONE
    } else if (block == Block::COBBLESTONE) {
        return computeAtlasBound({0, 16});
        // LEAF
    } else if (block == Block::LEAF) {
        return computeAtlasBound({64, 0});
        // CHERRY
    } else if (block == Block::CHERRY_LEAF) {
        return computeAtlasBound({80, 0});

        // DIAMOND
    } else if (block == Block::DIAMOND) {
        return computeAtlasBound({16, 16});
    } else {
        // In case everything else fails, just return dirt
        return computeAtlasBound({0, 0});
    }
}

float AtlasManager::getBlockSpecularStrength(Block block, Side side) const {
    if (block == Block::COBBLESTONE || block == Block::DIAMOND) {
        return 5.0f;
    } else {
        return 0.0f;
    }
}

glm::vec2 AtlasManager::convertIntCoords(glm::ivec2 coords) const {
    return {(float)coords.x / atlas.image.width,
            (float)coords.y / atlas.image.height};
}

AtlasManager::AtlasBounds AtlasManager::computeAtlasBound(
    glm::ivec2 coords) const {
    return {convertIntCoords(coords),
            convertIntCoords({coords.x + 16, coords.y + 16})};
}

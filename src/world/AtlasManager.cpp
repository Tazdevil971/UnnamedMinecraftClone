#include "AtlasManager.hpp"

using namespace world;

AtlasManager::AtlasManager(std::shared_ptr<render::TextureManager> textureMgr)
    : textureMgr{textureMgr} {
    atlas = textureMgr->createSimpleTexture("assets/block_atlas.png",
                                            VK_FORMAT_R8G8B8A8_SRGB);
}

void AtlasManager::cleanup() { textureMgr->deallocateSimpleTexture(atlas); }

AtlasManager::AtlasBounds AtlasManager::getAtlasBounds(Block block,
                                                       Side side) const {
    return AtlasBounds { convertIntCoords({0, 0}), convertIntCoords({16, 16}) };
}

glm::vec2 AtlasManager::convertIntCoords(glm::ivec2 coords) const {
    return {(float)coords.x / atlas.image.width,
            (float)coords.y / atlas.image.height};
}

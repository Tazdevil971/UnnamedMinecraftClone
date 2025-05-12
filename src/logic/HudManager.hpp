#pragma once

#include "../render/Renderer.hpp"
#include "../world/AtlasManager.hpp"
#include <list>
#include <memory>

namespace logic {
class HudManager {
public:
    static std::unique_ptr<HudManager> create(std::shared_ptr<world::AtlasManager> atlas) {
        return std::make_unique<HudManager>(atlas);
    }
    HudManager(std::shared_ptr<world::AtlasManager> atlas);
    void addToModelList(std::list<render::UiModel>& uiModels);
    void setSelectedBlock(int block) {
        selected_block = block;
    }

private:
    std::shared_ptr<world::AtlasManager> atlas;
    render::UiMesh pointerMesh;
    std::list<render::UiMesh> uiCubeMeshes;
    render::Texture pointerTexture;
    render::Texture cursorTexture;
    int selected_block;

};
}  // namespace logic
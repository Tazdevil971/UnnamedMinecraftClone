#include "HudManager.hpp"
#include "../render/BufferManager.hpp"

using namespace world;
using namespace render;
using namespace logic;

HudManager::HudManager(std::shared_ptr<AtlasManager> atlas) : atlas{atlas} {

    selected_block = 0;

    for (int i = 1; i < 8; i++) {
        std::vector<uint16_t> indices = {0, 1, 2, 3, 2, 1,  4, 5,  6,
                                         4, 6, 7, 8, 9, 10, 8, 10, 11};
        std::vector<UiVertex> vertices;
        // front
        auto bounds =
            atlas->getAtlasBounds(static_cast<Block>(i), Side::SIDE_X_POS);
        vertices.push_back({{-75, -75}, bounds.getTopLeft()});
        vertices.push_back({{-75, 75}, bounds.getBottomLeft()});
        vertices.push_back({{75, -75}, bounds.getTopRight()});
        vertices.push_back({{75, 75}, bounds.getBottomRight()});
        // side
        bounds = atlas->getAtlasBounds(static_cast<Block>(i), Side::SIDE_Z_POS);
        vertices.push_back({{75, -75}, bounds.getTopLeft()}),
            vertices.push_back({{75, 75}, bounds.getBottomLeft()});
        vertices.push_back({{100, 40}, bounds.getBottomRight()});
        vertices.push_back({{100, -110}, bounds.getTopRight()});
        // top
        bounds = atlas->getAtlasBounds(static_cast<Block>(i), Side::TOP);
        vertices.push_back({{-75, -75}, bounds.getTopRight()});
        vertices.push_back({{75, -75}, bounds.getBottomRight()});
        vertices.push_back({{100, -110}, bounds.getBottomLeft()});
        vertices.push_back({{-50, -110}, bounds.getTopLeft()});

        render::UiMesh uiCubeMesh =
            BufferManager::get().allocateMesh<UiMesh>(indices, vertices);
        uiCubeMeshes.push_back(std::move(uiCubeMesh));
    }

    pointerTexture = BufferManager::get().allocateTexture(
        "assets/pointer_texture.png", VK_FORMAT_R8G8B8A8_SRGB);

    pointerMesh = BufferManager::get().allocateMesh<UiMesh>(
        {0, 1, 2, 3, 2, 1}, {
                                {{-75, -75}, {0, 0}},
                                {{-75, 75}, {0, 1}},
                                {{75, -75}, {1, 0}},
                                {{75, 75}, {1, 1}},
                            });
    cursorTexture = BufferManager::get().allocateTexture(
        "assets/cursor.png", VK_FORMAT_R8G8B8A8_SRGB);
}

void HudManager::addToModelList(std::list<render::UiModel>& uiModels) {

    glm::vec2 center = {0, 0};
    uiModels.push_back(UiModel{pointerMesh, pointerTexture, center, CENTER});

    glm::vec2 pos = {-585, -200};
    for (auto const& mesh : uiCubeMeshes) {
        uiModels.push_back(
            UiModel{mesh, atlas->getAtlas(), pos, BOTTOM_CENTER});
        pos.x = pos.x + 200;
    }

    pos = {-585, -100};

    if (selected_block == 1) {
        uiModels.push_back(
            UiModel{pointerMesh, cursorTexture, pos, BOTTOM_CENTER});
    } else if (selected_block == 2) {
        pos = {-385, -100};
        uiModels.push_back(
            UiModel{pointerMesh, cursorTexture, pos, BOTTOM_CENTER});
    } else if (selected_block == 3) {
        pos = {-185, -100};
        uiModels.push_back(
            UiModel{pointerMesh, cursorTexture, pos, BOTTOM_CENTER});
    } else if (selected_block == 4) {
        pos = {15, -100};
        uiModels.push_back(
            UiModel{pointerMesh, cursorTexture, pos, BOTTOM_CENTER});
    } else if (selected_block == 5) {
        pos = {215, -100};
        uiModels.push_back(
            UiModel{pointerMesh, cursorTexture, pos, BOTTOM_CENTER});
    } else if (selected_block == 6) {
        pos = {415, -100};
        uiModels.push_back(
            UiModel{pointerMesh, cursorTexture, pos, BOTTOM_CENTER});
    } else if (selected_block == 7) {
        pos = {615, -100};
        uiModels.push_back(
            UiModel{pointerMesh, cursorTexture, pos, BOTTOM_CENTER});
    }
}
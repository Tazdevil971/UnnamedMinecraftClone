#include "Chunk.hpp"

#include <iostream>

#include "AtlasManager.hpp"

using namespace world;
using namespace render;

Chunk::Chunk() {
    for (int x = 0; x < DIM.x; x++) {
        for (int y = 0; y < DIM.y; y++) {
            for (int z = 0; z < DIM.z; z++) {
                blocks[x][y][z] = Block::AIR;
            }
        }
    }
}

void Chunk::cleanup() {
    if (!mesh.isNull()) {
        BufferManager::get().deallocateSimpleMeshDefer(mesh);
    }
}

Block Chunk::getBlock(glm::ivec3 pos) {
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    return blocks[x][y][z];
}

render::SimpleMesh Chunk::getMesh() { return mesh; }

render::SimpleModel Chunk::getModel(glm::ivec3 pos) {
    return SimpleModel{mesh, AtlasManager::get().getAtlas(), pos * DIM,
                       glm::vec3(0.0f, 0.0f, 0.0f)};
}

void Chunk::updateMesh() {
    std::vector<uint16_t> indices;
    std::vector<Vertex> vertices;

    for (int x = 0; x < DIM.x; x++) {
        for (int y = 0; y < DIM.y; y++) {
            for (int z = 0; z < DIM.z; z++) {
                if (blocks[x][y][z] != Block::AIR &&
                    (z == 0 || blocks[x][y][z - 1] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[x][y][z], Side::SIDE_Z_NEG);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x + 1, y, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{x, y, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{x, y + 1, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{x + 1, y + 1, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }
                if (blocks[x][y][z] != Block::AIR &&
                    (z == 15 || blocks[x][y][z + 1] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[x][y][z], Side::SIDE_Z_POS);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x, y, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{x + 1, y, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{x + 1, y + 1, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{x, y + 1, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }

                if (blocks[x][y][z] != Block::AIR &&
                    (y == 0 || blocks[x][y - 1][z] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[x][y][z], Side::BOTTOM);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x, y, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{x + 1, y, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{x + 1, y, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{x, y, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }

                if (blocks[x][y][z] != Block::AIR &&
                    (y == 15 || blocks[x][y + 1][z] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[x][y][z], Side::TOP);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x, y + 1, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{x + 1, y + 1, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{x + 1, y + 1, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{x, y + 1, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }

                if (blocks[x][y][z] != Block::AIR &&
                    (x == 0 || blocks[x - 1][y][z] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[x][y][z], Side::SIDE_X_NEG);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x, y, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{x, y, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{x, y + 1, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{x, y + 1, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }

                if (blocks[x][y][z] != Block::AIR &&
                    (x == 15 || blocks[x + 1][y][z] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[x][y][z], Side::SIDE_X_POS);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x + 1, y, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{x + 1, y, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{x + 1, y + 1, z},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{x + 1, y + 1, z + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }
            }
        }
    }

    if (!mesh.isNull()) {
        BufferManager::get().deallocateSimpleMeshDefer(mesh);
    }

    if (indices.size() > 0 || vertices.size() > 0) {
        mesh = BufferManager::get().allocateSimpleMesh(indices, vertices);
    } else {
        mesh = SimpleMesh{};
    }
}

Chunk Chunk::genChunk(glm::ivec3 pos) {
    Chunk chunk;
    for (int x = 0; x < DIM.x; x++) {
        for (int y = 0; y < DIM.y; y++) {
            for (int z = 0; z < DIM.z; z++) {
                if (y < 2) {
                    chunk.blocks[x][y][z] = Block::DIRT;
                } else if (y == 2) {
                    chunk.blocks[x][y][z] = Block::GRASS;
                } else {
                    chunk.blocks[x][y][z] = Block::AIR;
                }
            }
        }
    }
    chunk.updateMesh();
    return chunk;
}

void Chunk::updateBlock(glm::ivec3 pos, Block newBlock) {
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    blocks[x][y][z] = newBlock;
    updateMesh();
}
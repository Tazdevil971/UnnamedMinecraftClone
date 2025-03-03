#include "Chunk.hpp"

#include "AtlasManager.hpp"

#include <iostream>

using namespace world;
using namespace render;

Chunk::Chunk() {
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                blocks[x][y][z] = Block::AIR;
            }
        }
    }
}

void Chunk::cleanup() { BufferManager::get().deallocateSimpleMesh(mesh); }

Block Chunk::getBlock(glm::ivec3 pos) {
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    return blocks[x][y][z];
}

render::SimpleMesh Chunk::getMesh() { return mesh; }

void Chunk::updateMesh() {
    std::vector<uint16_t> indices;
    std::vector<Vertex> vertices;

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 16; k++) {
                if (blocks[i][j][k] != Block::AIR &&
                    (k == 0 || blocks[i][j][k - 1] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[i][j][k], Side::SIDE_Z_NEG);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{i + 1, j, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{i, j, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{i, j + 1, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{i + 1, j + 1, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }
                if (blocks[i][j][k] != Block::AIR &&
                    (k == 15 || blocks[i][j][k + 1] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[i][j][k], Side::SIDE_Z_POS);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{i, j, k+1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{i+1, j, k+1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{i+1, j + 1, k+1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{i , j + 1, k+1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }

                if (blocks[i][j][k] != Block::AIR &&
                    (j == 0 || blocks[i][j-1][k] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[i][j][k], Side::BOTTOM);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{i, j, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{i+1, j, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{i+1, j, k+1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{i, j, k+1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }

                if (blocks[i][j][k] != Block::AIR &&
                    (j == 15 || blocks[i][j+1][k] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[i][j][k], Side::TOP);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{i, j+1, k+1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{i + 1, j + 1, k + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{i + 1, j + 1, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{i, j + 1, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }

                 if (blocks[i][j][k] != Block::AIR &&
                    (i == 0 || blocks[i-1][j][k] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[i][j][k], Side::SIDE_X_NEG);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{i, j, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{i, j, k+1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{i, j+1, k + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{i, j+1, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }

                if (blocks[i][j][k] != Block::AIR &&
                    (i == 15 || blocks[i+1][j][k] == Block::AIR)) {
                    auto bounds = AtlasManager::get().getAtlasBounds(
                        blocks[i][j][k], Side::SIDE_X_POS);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{i+1, j, k + 1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomLeft()});
                    vertices.push_back({{i + 1, j, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getBottomRight()});
                    vertices.push_back({{i + 1, j + 1, k},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopRight()});
                    vertices.push_back({{i+1, j + 1, k+1},
                                        {0.0f, 0.0f, 0.0f},
                                        bounds.getTopLeft()});
                }
            }
        }
    }

    BufferManager::get().deallocateSimpleMesh(mesh);
    mesh = BufferManager::get().allocateSimpleMesh(indices, vertices);
}

Chunk Chunk::genChunk(glm::ivec3 pos) {
    Chunk chunk;
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
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
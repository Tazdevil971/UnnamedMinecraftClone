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

float noiseOctave(int worldX, int worldZ) {
    int octaves = 4;                 // Number of octaves
    float persistence = 0.7f;        // Controls amplitude reduction per octave
    float lacunarity = 1.5f;         // Controls frequency increase per octave
    float frequency = 1.0f / 100.0f;  // Base scale
    float amplitude = 1.0f;
    float total = 0.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += glm::simplex(glm::vec2(worldX, worldZ) * frequency) * amplitude;
        maxValue += amplitude;

        amplitude *= persistence;  // Reduce amplitude each octave
        frequency *= lacunarity;   // Increase frequency each octave
    }

    // Normalize noise value to [-1, 1]
    float noiseValue = total / maxValue;
    return noiseValue;
}

void Chunk::genTree(glm::ivec3 pos, Chunk &chunk, Block block){

    int x = pos.x;
    int y = pos.y;
    int z = pos.z;

    if (chunk.blocks[x][y - 1][z] == Block::COBBLESTONE ||
        chunk.blocks[x+1][y][z] == Block::WOOD_LOG ||
        chunk.blocks[x][y][z+1] == Block::WOOD_LOG ||
        chunk.blocks[x][y][z - 1] == Block::WOOD_LOG ||
        chunk.blocks[x-1][y][z] == Block::WOOD_LOG ||
        chunk.blocks[x - 1][y][z-1] == Block::WOOD_LOG ||
        chunk.blocks[x + 1][y][z+1] == Block::WOOD_LOG ||
        chunk.blocks[x + 1][y][z - 1] == Block::WOOD_LOG ||
        chunk.blocks[x - 1][y][z + 1] == Block::WOOD_LOG)
        {
        chunk.blocks[x][y][z] = Block::AIR;
        return;
    }

    while (y < 4) {
        chunk.blocks[x][y][z] = Block::WOOD_LOG;
        y++;
    }

    for (int k = 0; k < 2; k++) {
        for (int i = -1; i < 2; i++) {
            for (int j = -1; j < 2; j++) {
                if (block == Block::LEAF) {
                    chunk.blocks[x + i][y + k][z + j] = Block::LEAF;
                } else {
                    chunk.blocks[x + i][y + k][z + j] = Block::CHERRY_LEAF;
                }
            }
        }
    }
    chunk.blocks[x][y + 2][z] = Block::LEAF;
}


Chunk Chunk::genChunk(glm::ivec3 pos) {
    Chunk chunk;

    for (int x = 0; x < DIM.x; x++) {
        for (int z = 0; z < DIM.z; z++) {
            int worldX = pos.x * DIM.x + x;
            int worldZ = pos.z * DIM.z + z;
            // creates a noise value between -1 and 1 based on the global coordinates of the block
            float noiseValue = noiseOctave(worldX, worldZ);
            float treeNoise = glm::simplex(glm::vec2(worldX, worldZ)/50.0f);
            // maps the value in a range from 0 to 4*DIM.y
            int height = static_cast<int>((noiseValue + 1.0f) * 2*DIM.y);

            for (int y = 0; y < DIM.y; y++) {
                int worldY = pos.y * DIM.y + y;
                if (worldY < height && worldY > 2 * DIM.y) {
                    chunk.blocks[x][y][z] = Block::COBBLESTONE;
                } else if (worldY < (height-1)) {
                    chunk.blocks[x][y][z] = Block::DIRT;
                } else if (worldY == (height - 1)) {
                    chunk.blocks[x][y][z] = Block::GRASS;
                } else if (worldY == height && treeNoise<=-0.4f && treeNoise>=-0.6f && y<(DIM.y-7) && x<(DIM.x-1) && z<(DIM.z-1)){
                    if (static_cast<int>(treeNoise) % 2 == 0.0f) {
                        genTree({x, y, z}, chunk, Block::LEAF);
                    }
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
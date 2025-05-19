#include "Chunk.hpp"

#include "../render/BufferManager.hpp"
#include "../render/Constants.hpp"
#include "AtlasManager.hpp"
#include "Block.hpp"

using namespace world;
using namespace render;

Chunk::Chunk(std::shared_ptr<AtlasManager> atlas) : atlas{atlas} {
    for (int x = 0; x < DIM.x; x++) {
        for (int y = 0; y < DIM.y; y++) {
            for (int z = 0; z < DIM.z; z++) {
                blocks[x][y][z] = Block::AIR;
            }
        }
    }
}

Chunk::~Chunk() {
    if (!mesh.isNull()) {
        BufferManager::get().deallocateMeshDefer(std::move(mesh));
    }
}

Block Chunk::getBlock(glm::ivec3 pos) {
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    return blocks[x][y][z];
}

const render::GeometryMesh &Chunk::getMesh() { return mesh; }

render::GeometryModel Chunk::getModel(glm::ivec3 pos) {
    return GeometryModel{mesh, atlas->getAtlas(), pos * DIM,
                         glm::vec3(0.0f, 0.0f, 0.0f)};
}

void Chunk::updateMesh() {
    std::vector<uint16_t> indices;
    std::vector<GeometryVertex> vertices;

    for (int x = 0; x < DIM.x; x++) {
        for (int y = 0; y < DIM.y; y++) {
            for (int z = 0; z < DIM.z; z++) {
                Block block = blocks[x][y][z];
                if (block == Block::AIR) continue;

                if (z == 0 || blocks[x][y][z - 1] == Block::AIR) {
                    auto bounds =
                        atlas->getAtlasBounds(block, Side::SIDE_Z_NEG);
                    float spec = atlas->getBlockSpecularStrength(
                        block, Side::SIDE_Z_NEG);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x + 1, y, z},
                                        Z_NEG_NORMAL,
                                        bounds.getBottomLeft(),
                                        spec});
                    vertices.push_back({{x, y, z},
                                        Z_NEG_NORMAL,
                                        bounds.getBottomRight(),
                                        spec});
                    vertices.push_back({{x, y + 1, z},
                                        Z_NEG_NORMAL,
                                        bounds.getTopRight(),
                                        spec});
                    vertices.push_back({{x + 1, y + 1, z},
                                        Z_NEG_NORMAL,
                                        bounds.getTopLeft(),
                                        spec});
                }
                if (z == 15 || blocks[x][y][z + 1] == Block::AIR) {
                    auto bounds =
                        atlas->getAtlasBounds(block, Side::SIDE_Z_POS);
                    float spec = atlas->getBlockSpecularStrength(
                        block, Side::SIDE_Z_POS);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x, y, z + 1},
                                        Z_POS_NORMAL,
                                        bounds.getBottomLeft(),
                                        spec});
                    vertices.push_back({{x + 1, y, z + 1},
                                        Z_POS_NORMAL,
                                        bounds.getBottomRight(),
                                        spec});
                    vertices.push_back({{x + 1, y + 1, z + 1},
                                        Z_POS_NORMAL,
                                        bounds.getTopRight(),
                                        spec});
                    vertices.push_back({{x, y + 1, z + 1},
                                        Z_POS_NORMAL,
                                        bounds.getTopLeft(),
                                        spec});
                }

                if (y == 0 || blocks[x][y - 1][z] == Block::AIR) {
                    auto bounds =
                        atlas->getAtlasBounds(block, Side::SIDE_Y_NEG);
                    float spec = atlas->getBlockSpecularStrength(
                        block, Side::SIDE_Y_NEG);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x, y, z},
                                        Y_NEG_NORMAL,
                                        bounds.getBottomLeft(),
                                        spec});
                    vertices.push_back({{x + 1, y, z},
                                        Y_NEG_NORMAL,
                                        bounds.getBottomRight(),
                                        spec});
                    vertices.push_back({{x + 1, y, z + 1},
                                        Y_NEG_NORMAL,
                                        bounds.getTopRight(),
                                        spec});
                    vertices.push_back({{x, y, z + 1},
                                        Y_NEG_NORMAL,
                                        bounds.getTopLeft(),
                                        spec});
                }

                if (y == 15 || blocks[x][y + 1][z] == Block::AIR) {
                    auto bounds = atlas->getAtlasBounds(blocks[x][y][z],
                                                        Side::SIDE_Y_POS);
                    float spec = atlas->getBlockSpecularStrength(
                        block, Side::SIDE_Y_POS);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x, y + 1, z + 1},
                                        Y_POS_NORMAL,
                                        bounds.getBottomLeft(),
                                        spec});
                    vertices.push_back({{x + 1, y + 1, z + 1},
                                        Y_POS_NORMAL,
                                        bounds.getBottomRight(),
                                        spec});
                    vertices.push_back({{x + 1, y + 1, z},
                                        Y_POS_NORMAL,
                                        bounds.getTopRight(),
                                        spec});
                    vertices.push_back({{x, y + 1, z},
                                        Y_POS_NORMAL,
                                        bounds.getTopLeft(),
                                        spec});
                }

                if (x == 0 || blocks[x - 1][y][z] == Block::AIR) {
                    auto bounds = atlas->getAtlasBounds(blocks[x][y][z],
                                                        Side::SIDE_X_NEG);
                    float spec = atlas->getBlockSpecularStrength(
                        block, Side::SIDE_X_NEG);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x, y, z},
                                        X_NEG_NORMAL,
                                        bounds.getBottomLeft(),
                                        spec});
                    vertices.push_back({{x, y, z + 1},
                                        X_NEG_NORMAL,
                                        bounds.getBottomRight(),
                                        spec});
                    vertices.push_back({{x, y + 1, z + 1},
                                        X_NEG_NORMAL,
                                        bounds.getTopRight(),
                                        spec});
                    vertices.push_back({{x, y + 1, z},
                                        X_NEG_NORMAL,
                                        bounds.getTopLeft(),
                                        spec});
                }

                if (x == 15 || blocks[x + 1][y][z] == Block::AIR) {
                    auto bounds = atlas->getAtlasBounds(blocks[x][y][z],
                                                        Side::SIDE_X_POS);
                    float spec = atlas->getBlockSpecularStrength(
                        block, Side::SIDE_X_POS);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 1);
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size());
                    indices.push_back(vertices.size() + 2);
                    indices.push_back(vertices.size() + 3);
                    vertices.push_back({{x + 1, y, z + 1},
                                        X_POS_NORMAL,
                                        bounds.getBottomLeft(),
                                        spec});
                    vertices.push_back({{x + 1, y, z},
                                        X_POS_NORMAL,
                                        bounds.getBottomRight(),
                                        spec});
                    vertices.push_back({{x + 1, y + 1, z},
                                        X_POS_NORMAL,
                                        bounds.getTopRight(),
                                        spec});
                    vertices.push_back({{x + 1, y + 1, z + 1},
                                        X_POS_NORMAL,
                                        bounds.getTopLeft(),
                                        spec});
                }
            }
        }
    }

    if (!mesh.isNull()) {
        BufferManager::get().deallocateMeshDefer(std::move(mesh));
    }

    if (indices.size() > 0 || vertices.size() > 0) {
        mesh =
            BufferManager::get().allocateMesh<GeometryMesh>(indices, vertices);
    } else {
        mesh = GeometryMesh{};
    }
}

float noiseOctave(int worldX, int worldZ) {
    int octaves = 4;                  // Number of octaves
    float persistence = 0.7f;         // Controls amplitude reduction per octave
    float lacunarity = 1.5f;          // Controls frequency increase per octave
    float frequency = 1.0f / 100.0f;  // Base scale
    float amplitude = 1.0f;
    float total = 0.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total +=
            glm::simplex(glm::vec2(worldX, worldZ) * frequency) * amplitude;
        maxValue += amplitude;

        amplitude *= persistence;  // Reduce amplitude each octave
        frequency *= lacunarity;   // Increase frequency each octave
    }

    // Normalize noise value to [-1, 1]
    float noiseValue = total / maxValue;
    return noiseValue;
}

void Chunk::genTree(glm::ivec3 pos, Chunk &chunk) {
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;

    if (chunk.blocks[x + 1][y][z] == Block::WOOD_LOG ||
        chunk.blocks[x][y][z + 1] == Block::WOOD_LOG ||
        chunk.blocks[x][y][z - 1] == Block::WOOD_LOG ||
        chunk.blocks[x - 1][y][z] == Block::WOOD_LOG ||
        chunk.blocks[x - 1][y][z - 1] == Block::WOOD_LOG ||
        chunk.blocks[x + 1][y][z + 1] == Block::WOOD_LOG ||
        chunk.blocks[x + 1][y][z - 1] == Block::WOOD_LOG ||
        chunk.blocks[x - 1][y][z + 1] == Block::WOOD_LOG) {
        chunk.blocks[x][y][z] = Block::AIR;
        return;
    }


   for (int i = 0; i < 5; i++) {
        chunk.blocks[x][y+i][z] = Block::WOOD_LOG;
   }
   y = y+4;

    for (int k = 0; k < 2; k++) {
        for (int i = -1; i < 2; i++) {
            for (int j = -1; j < 2; j++) {
                chunk.blocks[x + i][y + k][z + j] = Block::LEAF;
            }
        }
    }
    chunk.blocks[x][y + 2][z] = Block::LEAF;
}

float whiteNoise(int worldX, int worldZ) {
    uint32_t hash = std::hash<int>{}(worldX) ^ std::hash<int>{}(worldZ);
    return static_cast<float>(hash) / static_cast<float>(UINT32_MAX);
}

Chunk Chunk::genChunk(std::shared_ptr<AtlasManager> atlas, glm::ivec3 pos) {
    Chunk chunk{atlas};

    for (int x = 0; x < DIM.x; x++) {
        for (int z = 0; z < DIM.z; z++) {
            int worldX = pos.x * DIM.x + x;
            int worldZ = pos.z * DIM.z + z;
            // creates a noise value between -1 and 1 based on the global
            // coordinates of the block
            float noiseValue = noiseOctave(worldX, worldZ);
            float treeNoise = glm::simplex(glm::vec2(worldX, worldZ) / 50.0f);
            float treeProbability =  whiteNoise(worldX, worldZ);
            // maps the value in a range from 0 to 4*DIM.y
            int height = static_cast<int>((noiseValue + 1.0f) * 2 * DIM.y);

            for (int y = 0; y < DIM.y; y++) {
                int worldY = pos.y * DIM.y + y;
                if (worldY < height && worldY > 2 * DIM.y) {
                    chunk.blocks[x][y][z] = Block::COBBLESTONE;
                } else if (worldY < (height - 1)) {
                    chunk.blocks[x][y][z] = Block::DIRT;
                } else if (worldY == (height - 1)) {
                    chunk.blocks[x][y][z] = Block::GRASS;
                    if (treeNoise > treeProbability && treeNoise>0.4f && y < (DIM.y - 8) && x < (DIM.x - 1) && z > 1 && 
                        z < (DIM.z - 1) && z >1) {
                        genTree({x, y+1, z}, chunk);
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
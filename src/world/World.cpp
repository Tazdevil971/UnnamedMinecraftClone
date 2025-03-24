#include "World.hpp"

#include <iostream>

using namespace world;
using namespace render;

World::World() {}

void World::cleanup() {
    for (auto& it : chunks) {
        it.second.cleanup();
    }

    chunks.clear();
}

Chunk& World::getChunk(glm::ivec3 pos) {
    // Get an existing chunk or create a new one
    auto it = chunks.find(pos);
    if (it == chunks.end()) {
        it = chunks.insert({pos, Chunk::genChunk(pos)}).first;
        return it->second;
    } else {
        return it->second;
    }
}

Block World::getBlock(glm::ivec3 pos) {
    auto [chunkPos, inChunkPos] = splitWorldCoords(pos);
    if (chunkPos.y < 0 || chunkPos.y >= HEIGHT)
        return Block::AIR;

    return getChunk(chunkPos).getBlock(inChunkPos);
}

void World::updateBlock(glm::ivec3 pos, Block newBlock) {
    auto [chunkPos, inChunkPos] = splitWorldCoords(pos);
    if (chunkPos.y < 0 || chunkPos.y >= HEIGHT)
        return;

    getChunk(chunkPos).updateBlock(inChunkPos, newBlock);
}

// Division towards "bottom"
int fixedDiv(int value, int div) {
    return value >= 0 ? (value / div) : ((value - div + 1) / div);
}

int fixedMod(int value, int mod) {
    return value >= 0 ? (value % mod) : (((value % mod) + mod) % mod);
}

std::pair<glm::ivec3, glm::ivec3> World::splitWorldCoords(glm::ivec3 pos) {
    glm::ivec3 chunkPos{fixedDiv(pos.x, Chunk::DIM.x),
                        fixedDiv(pos.y, Chunk::DIM.y),
                        fixedDiv(pos.z, Chunk::DIM.z)};
    glm::ivec3 inChunkPos{fixedMod(pos.x, Chunk::DIM.x),
                          fixedMod(pos.y, Chunk::DIM.y),
                          fixedMod(pos.z, Chunk::DIM.z)};

    return {chunkPos, inChunkPos};
}

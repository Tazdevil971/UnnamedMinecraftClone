#include "World.hpp"

#include <cmath>
#include <glm/geometric.hpp>

World::World() {}

Chunk World::createChunk(glm::ivec3 pos) {
    Chunk chunk = Chunk::genChunk(pos);
    chunks[pos] = chunk;
    return chunk;
}

Chunk& World::getChunk(glm::ivec3 pos) { return chunks[pos]; }

std::list<Chunk> World::getChunkInArea(glm::ivec3 pos, float radius) {
    std::list<Chunk> chunksInArea;
    for (auto& [key, value] : chunks) {
        if (glm::distance(pos, key) <= radius) {
            chunksInArea.push_back(value);
        }
    }
    return chunksInArea;
}

Block World::getBlock(glm::ivec3 pos) {
    Chunk chunk = getChunk(pos);
    Block block = chunk.getBlock(pos / 16);
    return block;
}

void World::updateBlock(glm::ivec3 pos, Block newBlock) {
    Chunk chunk = getChunk(pos);
    Block block = chunk.getBlock(pos / 16);
    block = newBlock;
    return;
}
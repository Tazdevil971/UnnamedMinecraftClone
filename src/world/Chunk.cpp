#include "Chunk.hpp"

Chunk::Chunk() {}


Block Chunk::getBlock(glm::ivec3 pos) { 
	int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    return blocks[x][y][z];
}

/* GpuBuffer Chunk::getMesh() { 
    return buffer; 
}*/

static Chunk Chunk::genChunk(glm::ivec3 pos) {
    Chunk chunk;
    for(int x = 0; x < 16; x++) {
        for(int y = 0; y < 16; y++) {
            for(int z = 0; z < 16; z++) {
                if(y < 2) {
                    chunk.blocks[x][y][z] = Block::DIRT;
                } else if(y == 2) {
                    chunk.blocks[x][y][z] = Block::GRASS;
                } else chunk.blocks[x][y][z] = Block::AIR;
            }
        }
    }
    return chunk;
}

void Chunk::updateBlock(glm::ivec3 pos, Block newBlock, vk::BufferManager* bufMgr) {
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    blocks[x][y][z] = newBlock;
    return;
}
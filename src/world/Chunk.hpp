#pragma once
#include <glm/vec3.hpp>
#include <array>
#include "Block.hpp"

class Chunk {
	private:
		Block blocks[16][16][16];
        //GpuBuffer buffer;
	public:
        Chunk();
        Block getBlock(glm::ivec3 pos);
        //GpuBuffer getMesh();
        static Chunk genChunk(glm::ivec3 pos);
        void updateBlock(glm::ivec3 pos, Block newBlock, vk::BufferManager *bufMgr);
};
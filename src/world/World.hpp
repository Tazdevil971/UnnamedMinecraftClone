#pragma once
#include "Chunk.hpp"
#include <map>
#include <list>
#include <glm/vec3.hpp>

class World {
   private:
       vk::BufferManager *bufMgr;
       std::map<glm::ivec3, Chunk> chunks;
       Chunk createChunk(glm::ivec3 pos);
   public:
       World();
       Chunk getChunk(glm::ivec3 pos);
       std::list<Chunk> getChunkInArea(glm::ivec3 pos, float radius);
       Block getBlock(glm::ivec3 pos);
       void updateBlock(glm::ivec3 pos, Block newBlock);
};
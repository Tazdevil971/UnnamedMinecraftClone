#pragma once
#include <array>
#include <glm/vec3.hpp>

#include "Block.hpp"
#include "../render/Mesh.hpp"
#include "../render/BufferManager.hpp"

class Chunk {
   private:
    Block blocks[16][16][16];
    render::SimpleMesh mesh;
   public:
    Chunk();
    Block getBlock(glm::ivec3 pos);
    render::SimpleMesh getMesh();
    static Chunk genChunk(glm::ivec3 pos, render::BufferManager *bufMgr);
    void updateBlock(glm::ivec3 pos, Block newBlock, render::BufferManager *bufMgr);
private:
    void updateMesh(render::BufferManager *bufMgr);
};
#pragma once
#include <array>
#include <glm/vec3.hpp>

#include "Block.hpp"
#include "../render/Mesh.hpp"
#include "../render/BufferManager.hpp"

namespace world {

class Chunk {
   private:
    Block blocks[16][16][16];
    render::SimpleMesh mesh;
    void updateMesh();

   public:
    Chunk();
    void cleanup();
    Block getBlock(glm::ivec3 pos);
    render::SimpleMesh getMesh();
    static Chunk genChunk(glm::ivec3 pos);
    void updateBlock(glm::ivec3 pos, Block newBlock);
};

}  // namespace world
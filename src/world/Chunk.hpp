#pragma once
#include <array>
#include <glm/vec3.hpp>

#include "../render/BufferManager.hpp"
#include "../render/Mesh.hpp"
#include "Block.hpp"

namespace world {

class Chunk {
   public:
    static constexpr glm::ivec3 DIM = glm::ivec3(16, 16, 16);

   private:
    Block blocks[DIM.x][DIM.y][DIM.z];
    render::SimpleMesh mesh;

    void updateMesh();

   public:
    Chunk();
    void cleanup();

    static Chunk genChunk(glm::ivec3 pos);

    Block getBlock(glm::ivec3 pos);
    void updateBlock(glm::ivec3 pos, Block newBlock);

    render::SimpleMesh getMesh();
    render::SimpleModel getModel(glm::ivec3 pos);
};

}  // namespace world
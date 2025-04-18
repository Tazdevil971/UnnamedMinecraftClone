#pragma once
#include <glm/gtc/noise.hpp>
#include <glm/vec3.hpp>

#include "../render/Primitives.hpp"
#include "Block.hpp"


namespace world {

class Chunk {
public:
    static constexpr glm::ivec3 DIM = glm::ivec3(16, 16, 16);

private:
    Block blocks[DIM.x][DIM.y][DIM.z];
    render::GeometryMesh mesh;

    void updateMesh();
    static void genTree(glm::ivec3 pos, Chunk &chunk, Block block);

public:
    Chunk();
    void cleanup();

    static Chunk genChunk(glm::ivec3 pos);

    Block getBlock(glm::ivec3 pos);
    void updateBlock(glm::ivec3 pos, Block newBlock);

    render::GeometryMesh getMesh();
    render::GeometryModel getModel(glm::ivec3 pos);
};

}  // namespace world
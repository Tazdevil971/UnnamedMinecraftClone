#pragma once
#include <glm/gtc/noise.hpp>
#include <glm/vec3.hpp>
#include <memory>

#include "../render/Primitives.hpp"
#include "AtlasManager.hpp"
#include "Block.hpp"

namespace world {

class Chunk {
public:
    static constexpr glm::ivec3 DIM = glm::ivec3(16, 16, 16);

private:
    Block blocks[DIM.x][DIM.y][DIM.z];
    render::GeometryMesh mesh;

    std::shared_ptr<AtlasManager> atlas;

    void updateMesh();
    static void genTree(glm::ivec3 pos, Chunk &chunk);

public:
    Chunk(std::shared_ptr<AtlasManager> atlas);
    ~Chunk();

    Chunk(Chunk &&) = default;
    Chunk &operator=(Chunk &&) = default;

    static Chunk genChunk(std::shared_ptr<AtlasManager> atlas, glm::ivec3 pos);

    Block getBlock(glm::ivec3 pos);
    void updateBlock(glm::ivec3 pos, Block newBlock);

    const render::GeometryMesh &getMesh();
    render::GeometryModel getModel(glm::ivec3 pos);
};

}  // namespace world
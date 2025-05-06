#pragma once
#include <glm/vec3.hpp>
#include <memory>
#include <unordered_map>

#include "AtlasManager.hpp"
#include "Chunk.hpp"

namespace world {

struct IVec3Hash {
    static void hash_combine(std::size_t& seed, int v) {
        seed ^= std::hash<int>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    std::size_t operator()(const glm::ivec3& vec) const {
        std::size_t seed = 0;
        hash_combine(seed, vec.x);
        hash_combine(seed, vec.y);
        hash_combine(seed, vec.z);
        return seed;
    }
};

class World {
public:
    static constexpr int HEIGHT = 4;

private:
    std::unordered_map<glm::ivec3, Chunk, IVec3Hash> chunks;
    std::shared_ptr<AtlasManager> atlas;

public:
    World(std::shared_ptr<AtlasManager> atlas);

    static std::unique_ptr<World> create(std::shared_ptr<AtlasManager> atlas) {
        return std::make_unique<World>(atlas);
    }

    template <typename F>
    void getChunkInArea(glm::ivec3 pos, int radius, const F& f);

    Chunk& getChunk(glm::ivec3 pos);
    Block getBlock(glm::ivec3 pos);
    void updateBlock(glm::ivec3 pos, Block newBlock);
    static std::pair<glm::ivec3, glm::ivec3> splitWorldCoords(glm::ivec3 pos);
};

template <typename F>
void World::getChunkInArea(glm::ivec3 pos, int radius, const F& f) {
    // Get pos in chunk coord system
    pos /= Chunk::DIM;

    for (int x = -radius; x < radius; x++) {
        for (int z = -radius; z < radius; z++) {
            for (int y = 0; y < HEIGHT; y++) {
                glm::ivec3 pos2{pos.x + x, y, pos.z + z};
                Chunk& chunk = getChunk(pos2);
                f(pos2, chunk);
            }
        }
    }
}

}  // namespace world
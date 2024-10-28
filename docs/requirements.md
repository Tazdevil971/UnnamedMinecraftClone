# Requirements
Requirements and goals of the project.

## Basic goals
- Simple terrain generation
- Be able to place and break blocks
  - 2-3 different available
- Simple physics
- Simple raycasting for block placement/breakage
- Rendering chunks with diffuse lighting model

## Optional goals
- Skybox rendering
- Shadow mapping
- Saving/loading maps

## Very optional goals
- NPCs? (le mucchine)

## UML
```plantuml
@startuml
  enum Block {
    AIR
    GRASS
    DIRT
    COBBLESTONE
  }
  
  class ChunkMesh {}

  class Chunk {
    - Block blocks[16][16][16]
    + Vec3d getOrigin()
    + Block getBlock(Vec3d pos)
    + void updateBlock(Vec3d pos, Block newBlock)
    + ChunkMesh getMesh()
    + static Chunk genChunk(float yLevel)
  }

  World --* Chunk

  class World {
    - map<Vec3d, Chunk>
    + Chunk getChunk(Vec3d pos)
    + list<Chunk> getChunkInArea(Vec3d pos, float radius)
    + Block getBlock(Vec3d pos)
    + void updateBlock(Vec3d pos, Block newBlock)
  }

  class PlayerController {
    - World* world
    - Renderer* renderer
    + void update()
    + void render()
  }

  class Renderer {
    + void render(Vec3d cameraPos, list<Chunk> chunks)
  }
@enduml
```
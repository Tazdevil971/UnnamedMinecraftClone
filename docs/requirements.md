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
  
  class vk::Context {
    - GLFWwindow *window
    + VkDevice device()
    + VkInstance instance()
    + VkQueue graphicsQueue()
    + VkQueue presentQueue()
    + VkQueue transferQueue()
  }

  class vk::BufferManager {
    - vk::Context *ctx
    + GpuBuffer createBuffer<T>(const T* data)
    + GpuImage createImage(...)
    + GpuImage loadImage(...)
  }

  class vk::Framebuffer {
    - vk::Context *ctx
    + void recreate()
    + ...
  }

  class vk::RenderSubpass {
    + ...
  }

  class vk::Renderer {
    - vk::Context *ctx
    - vk::Framebuffer *fb
    + void render(glm::vec3 cameraPos, list<Chunk> chunks)
  }

  class AtlasBounds {
    + glm::vec2 topLeft;
    + glm::vec2 bottomRight;
  }

  enum Side {
    TOP, BOTTOM, SIDE
  }

  class TextureAtlasManager {
    - vk::Texture atlas;
    + vk::Texture &getAtlas();
    + AtlasBounds getBounds(Block block, Side side);
  }

  class Chunk {
    - Block blocks[16][16][16]
    - GpuBuffer buffer
    + Block getBlock(glm::ivec3 pos)
    + GpuBuffer getMesh()
    + static Chunk genChunk(glm::ivec3 pos)
    + void updateBlock(glm::ivec3 pos,vk::BufferManager *bufMgr)
  }

  World --* Chunk

  class World {
    - vk::BufferManager *bufMgr
    - map<glm::ivec3, Chunk> chunks
    - Chunk createChunk(glm::ivec3 pos)
    + Chunk getChunk(glm::ivec3 pos)
    + list<Chunk> getChunkInArea(glm::ivec3 pos, float radius)
    + Block getBlock(glm::ivec3 pos)
    + void updateBlock(glm::ivec3 pos, Block newBlock)
  }

  class PlayerController {
    - World* world
    - vk::Renderer* renderer
    + void update()
    + void render()
  }
@enduml
```
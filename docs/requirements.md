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

  class Chunk {
    - Block blocks[16][16][16]
    - GpuBuffer buffer
    + glm::vec3 getOrigin()
    + Block getBlock(glm::vec3 pos)
    + GpuBuffer getMesh()
    + static Chunk genChunk(float yLevel)
  }

  World --* Chunk

  class World {
    - vk::BufferManager *bufMgr
    - map<glm::vec3, Chunk>
    - Chunk createChunk(glm::vec3 pos)
    + Chunk getChunk(glm::vec3 pos)
    + list<Chunk> getChunkInArea(glm::vec3 pos, float radius)
    + Block getBlock(glm::vec3 pos)
    + void updateBlock(glm::vec3 pos, Block newBlock)
  }

  class PlayerController {
    - World* world
    - vk::Renderer* renderer
    + void update()
    + void render()
  }
@enduml
```
cmake_minimum_required(VERSION 3.20)
include(cmake/AddShaders.cmake)
include(cmake/AddAssets.cmake)

add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)
add_subdirectory(libs/backward-cpp)
add_subdirectory(libs/stb_image)
add_subdirectory(libs/VulkanMemoryAllocator)

project(UnnamedMinecraftClone LANGUAGES CXX)

find_package(Vulkan REQUIRED)

set(SOURCES 
    src/main.cpp 
    src/MainWindow.cpp
    src/render/Context.cpp
    src/render/Swapchain.cpp
    src/render/Framebuffer.cpp
    src/render/BufferManager.cpp
    src/render/VmaImplementation.cpp
    src/render/Renderer.cpp
    src/render/ShadowPass.cpp
    src/render/ForwardPass.cpp
    src/render/SkyboxRenderer.cpp
    src/render/GeometryRenderer.cpp
    src/render/UiRenderer.cpp
    src/render/Skybox.cpp
    src/render/Window.cpp
    src/world/AtlasManager.cpp
    src/world/World.cpp
    src/world/Chunk.cpp
    src/world/AnimModel.cpp
    src/world/Mucchina.cpp
    src/logic/Collision.cpp
    src/logic/PlayerController.cpp
    src/logic/VoxelRaytracer.cpp
    src/logic/DayNightCycle.cpp
    )

set(SHADERS
    src/shaders/ShadowVert.vert
    src/shaders/ShadowFrag.frag
    src/shaders/SkyboxVert.vert
    src/shaders/SkyboxFrag.frag
    src/shaders/GeometryVert.vert
    src/shaders/GeometryFrag.frag
    src/shaders/UiVert.vert
    src/shaders/UiFrag.frag
    )

set(ASSETS
    assets/test_image.jpg
    assets/block_atlas.png
    assets/pointer_texture.png
    assets/skybox_day.png
    assets/skybox_night.png
    assets/debug.png
    assets/debug_skybox.png
    assets/pointer_texture.png
    assets/cursor.png
    assets/mucchina.png
    )

add_executable(UnnamedMinecraftClone ${SOURCES})
target_compile_features(UnnamedMinecraftClone PRIVATE cxx_std_17)

add_shaders(UnnamedMinecraftClone_shaders ${SHADERS})
add_assets(UnnamedMinecraftClone_assets ${ASSETS})
    
target_include_directories(UnnamedMinecraftClone PUBLIC ${Vulkan_INCLUDE_DIRS})
target_compile_definitions(
    UnnamedMinecraftClone 
    PRIVATE 
    _USE_MATH_DEFINES
    GLM_ENABLE_EXPERIMENTAL
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_link_libraries(
    UnnamedMinecraftClone
    PRIVATE
    Backward::Interface
    glfw
    glm::glm
    stb_image
    GPUOpen::VulkanMemoryAllocator
    ${Vulkan_LIBRARIES})
add_dependencies(
    UnnamedMinecraftClone 
    UnnamedMinecraftClone_shaders 
    UnnamedMinecraftClone_assets)
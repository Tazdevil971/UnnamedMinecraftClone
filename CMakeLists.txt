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
    # src/Window2.cpp
    src/Window.cpp
    src/render/Context.cpp
    src/render/Swapchain.cpp
    src/render/Framebuffer.cpp
    src/render/BufferManager.cpp
    src/render/TextureManager.cpp
    src/render/VmaImplementation.cpp
    src/render/Renderer.cpp
    # src/world/World.cpp
    # src/world/Chunk.cpp
    )

set(SHADERS
    src/shaders/SimpleVert.vert
    src/shaders/SimpleFrag.frag)

set(ASSETS
    assets/test_image.jpg)

add_executable(UnnamedMinecraftClone ${SOURCES})
target_compile_features(UnnamedMinecraftClone PRIVATE cxx_std_17)

add_shaders(UnnamedMinecraftClone_shaders ${SHADERS})
add_assets(UnnamedMinecraftClone_assets ${ASSETS})
    
target_include_directories(UnnamedMinecraftClone PUBLIC ${Vulkan_INCLUDE_DIRS})
target_compile_definitions(
    UnnamedMinecraftClone 
    PRIVATE 
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
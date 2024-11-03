cmake_minimum_required(VERSION 3.20)
include(cmake/AddShaders.cmake)

add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)

find_package(Vulkan REQUIRED)

project(UnnamedMinecrafClone LANGUAGES CXX)

set(SOURCES 
    src/main.cpp 
    src/Window.cpp)

set(SHADERS
    src/shaders/SimpleVert.vert
    src/shaders/SimpleFrag.frag)

add_executable(UnnamedMinecrafClone ${SOURCES})
target_compile_features(UnnamedMinecrafClone PRIVATE cxx_std_17)

add_shaders(UnnamedMinecrafClone_shaders ${SHADERS})
    
target_include_directories(UnnamedMinecrafClone PUBLIC ${Vulkan_INCLUDE_DIRS})
target_compile_definitions(UnnamedMinecrafClone PRIVATE GLM_FORCE_RADIANS)
target_link_libraries(
    UnnamedMinecrafClone
    glfw
    glm::glm
    ${Vulkan_LIBRARIES})
add_dependencies(UnnamedMinecrafClone UnnamedMinecrafClone_shaders)
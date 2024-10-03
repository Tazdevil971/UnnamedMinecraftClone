cmake_minimum_required(VERSION 3.15)

project(UnnamedMinecrafClone LANGUAGES CXX)

add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)

find_package(Vulkan REQUIRED)

set(SOURCES 
    src/main.cpp 
    src/Window.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(
    ${PROJECT_NAME} 
    glfw
    glm::glm
    ${Vulkan_LIBRARIES})
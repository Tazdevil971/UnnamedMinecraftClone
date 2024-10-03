cmake_minimum_required(VERSION 3.15)

project(UnnamedMinecrafClone LANGUAGES CXX)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

set(SOURCES 
    src/main.cpp 
    src/Window.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
cmake_minimum_required(VERSION 3.20)

function(add_shaders TARGET_NAME)
  set(SHADER_SOURCE_FILES ${ARGN})

  set(OUTPUTS)
  foreach(SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
    cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
    cmake_path(GET SHADER_SOURCE FILENAME SHADER_NAME)

    set(SHADER_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv")

    add_custom_command(
        OUTPUT "${SHADER_OUTPUT}"
        COMMAND "${Vulkan_GLSLC_EXECUTABLE}" ARGS "${SHADER_SOURCE}" -o "${SHADER_OUTPUT}"
        DEPENDS "${SHADER_SOURCE}"
        COMMENT "Building shader ${SHADER_SOURCE}")
    
    list(APPEND OUTPUTS "${SHADER_OUTPUT}")
  endforeach()

  add_custom_target(${TARGET_NAME} DEPENDS ${OUTPUTS})
endfunction()
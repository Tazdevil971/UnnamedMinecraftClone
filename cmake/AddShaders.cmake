cmake_minimum_required(VERSION 3.20)

function(add_shaders TARGET_NAME)
    set(SHADER_SOURCE_FILES ${ARGN})
  
    set(OUTPUTS)
    foreach(SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
        cmake_path(
            ABSOLUTE_PATH 
            SHADER_SOURCE 
            NORMALIZE 
            OUTPUT_VARIABLE 
            SHADER_SOURCE_ABS)
        cmake_path(GET SHADER_SOURCE_ABS FILENAME SHADER_NAME)
  
        set(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv")
  
        add_custom_command(
            OUTPUT "${OUTPUT}"
            COMMAND "${Vulkan_GLSLC_EXECUTABLE}" ARGS "${SHADER_SOURCE_ABS}" -o "${OUTPUT}"
            DEPENDS "${SHADER_SOURCE_ABS}"
            COMMENT "Building shader ${SHADER_SOURCE}")
        
        list(APPEND OUTPUTS "${OUTPUT}")
    endforeach()
  
    add_custom_target(${TARGET_NAME} DEPENDS ${OUTPUTS})
endfunction()
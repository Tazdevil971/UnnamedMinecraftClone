cmake_minimum_required(VERSION 3.20)

function(add_assets TARGET_NAME)
    set(ASSET_SOURCE_FILES ${ARGN})
  
    set(OUTPUTS)
    foreach(ASSET_SOURCE IN LISTS ASSET_SOURCE_FILES)
        cmake_path(
            ABSOLUTE_PATH 
            ASSET_SOURCE 
            NORMALIZE 
            OUTPUT_VARIABLE 
            ASSET_SOURCE_ABS)
    
        set(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${ASSET_SOURCE}")
    
        add_custom_command(
            OUTPUT "${OUTPUT}"
            COMMAND "${CMAKE_COMMAND}" ARGS -E copy "${ASSET_SOURCE_ABS}" "${OUTPUT}"
            DEPENDS "${ASSET_SOURCE_ABS}"
            COMMENT "Copying ${ASSET_SOURCE}")
        
        list(APPEND OUTPUTS "${OUTPUT}")
    endforeach()
  
    add_custom_target(${TARGET_NAME} DEPENDS ${OUTPUTS})
endfunction()
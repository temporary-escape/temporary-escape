find_program(GLSL_COMPILER "glslc" PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools")
if (NOT GLSL_COMPILER)
    message(FATAL_ERROR "Failed to find glslc in the vcpkg folder")
endif ()

set(FILE_TO_SOURCE "${CMAKE_CURRENT_LIST_DIR}/FileToSource.cmake")

function(compile_shaders NAME DIRECTORY)
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}")
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")

    file(GLOB SHADERS "${DIRECTORY}/*.glsl")
    file(GLOB INCLUDES "${DIRECTORY}/includes/*.glsl")

    foreach (FILE ${SHADERS})
        get_filename_component(BASENAME ${FILE} NAME_WE)

        if (BASENAME MATCHES "_frag$")
            set(SHADER_TYPE "frag")
        elseif (BASENAME MATCHES "_vert$")
            set(SHADER_TYPE "vert")
        elseif (BASENAME MATCHES "_comp$")
            set(SHADER_TYPE "comp")
        else ()
            message(FATAL_ERROR "Unknown shader type: ${BASENAME}")
        endif ()

        set(SPIRV_FILE "${OUTPUT_DIR}/${BASENAME}.spirv")

        add_custom_command(OUTPUT ${SPIRV_FILE}
                COMMAND ${GLSL_COMPILER} -fshader-stage=${SHADER_TYPE} "${FILE}" -x glsl --target-env=vulkan1.2 -o "${SPIRV_FILE}"
                WORKING_DIRECTORY ${OUTPUT_DIR}
                DEPENDS ${FILE} ${INCLUDES}
                VERBATIM
        )

        add_custom_command(OUTPUT ${SPIRV_FILE}.h
                COMMAND ${CMAKE_COMMAND} "-DINPUT=${SPIRV_FILE}" "-DOUTPUT=${SPIRV_FILE}.h" -P "${FILE_TO_SOURCE}"
                WORKING_DIRECTORY ${OUTPUT_DIR}
                DEPENDS ${SPIRV_FILE}
                VERBATIM
        )

        list(APPEND SPIRV_FILES "${SPIRV_FILE}.h")
    endforeach ()

    add_custom_target(${NAME} ALL
            COMMENT "Compile shaders"
            DEPENDS ${SPIRV_FILES}
    )

    set_target_properties(${NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${OUTPUT_DIR}")
endfunction()

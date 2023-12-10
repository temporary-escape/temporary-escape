set(FILE_TO_SOURCE "${CMAKE_CURRENT_LIST_DIR}/FileToSource.cmake")

function(embed_files NAME DIRECTORY)
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}")
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")

    file(GLOB FILES "${DIRECTORY}")

    foreach (FILE ${FILES})
        get_filename_component(FILENAME ${FILE} NAME)

        set(OUTPUT_FILE "${OUTPUT_DIR}/${FILENAME}.h")

        add_custom_command(OUTPUT ${OUTPUT_FILE}
                COMMAND ${CMAKE_COMMAND} "-DINPUT=${FILE}" "-DOUTPUT=${OUTPUT_FILE}" -P "${FILE_TO_SOURCE}"
                WORKING_DIRECTORY ${OUTPUT_DIR}
                DEPENDS ${FILE}
                VERBATIM
        )

        list(APPEND OUTPUT_FILES "${OUTPUT_FILE}")
    endforeach ()

    add_custom_target(${NAME} ALL DEPENDS ${OUTPUT_FILES})

    set_target_properties(${NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${OUTPUT_DIR}")
endfunction()

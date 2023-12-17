if (NOT INPUT)
    message(FATAL_ERROR "No input defined")
endif ()

if (NOT OUTPUT)
    message(FATAL_ERROR "No output defined")
endif ()

file(READ ${INPUT} HEX_CONTENT HEX)

get_filename_component(FILENAME ${INPUT} NAME)
string(REPLACE "." "_" VARIABLE_NAME "${FILENAME}")
string(REPLACE "-" "_" VARIABLE_NAME "${VARIABLE_NAME}")

string(REPEAT "[0-9a-f]" 32 COLUMN_PATTERN)
string(REGEX REPLACE "(${COLUMN_PATTERN})" "\\1\n" CONTENT "${HEX_CONTENT}")
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1, " CONTENT "${CONTENT}")
string(REGEX REPLACE ", $" "" CONTENT "${CONTENT}")
set(ARRAY_DEFINITION "static const inline uint8_t ${VARIABLE_NAME}[] =\n{\n${CONTENT}\n};")
set(SOURCE "// Auto generated from file: ${INPUT}\nnamespace Engine::Embed {\n${ARRAY_DEFINITION}\n}\n")
file(WRITE "${OUTPUT}" "${SOURCE}")

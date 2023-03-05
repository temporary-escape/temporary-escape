execute_process(COMMAND git describe --always --tags
        OUTPUT_VARIABLE VERSION_STRING
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE VERSION_STRING_RET)

if (NOT VERSION_STRING_RET EQUAL "0")
    set(VERSION_STRING "v0.0.1")
endif ()

message(STATUS "Project version: ${VERSION_STRING}")

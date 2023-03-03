install(DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "${CMAKE_CURRENT_SOURCE_DIR}/shapes"
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        DESTINATION ".")

install(DIRECTORY
        "${CPYTHON_STDLIB_DIR}/"
        DESTINATION "python"
        PATTERN "__pycache__" EXCLUDE)

if (MSVC)
    install(DIRECTORY
            "${CPYTHON_DLLS_DIR}/"
            DESTINATION ".")

    install(DIRECTORY
            "${CMAKE_BINARY_DIR}/"
            DESTINATION "."
            FILES_MATCHING PATTERN "*.dll"
            PATTERN "src" EXCLUDE
            PATTERN "test" EXCLUDE
            PATTERN "vcpkg_installed" EXCLUDE
            PATTERN "python" EXCLUDE
            PATTERN "CMakeFiles" EXCLUDE
            PATTERN "install" EXCLUDE
            PATTERN "release" EXCLUDE
            PATTERN "_CPack_Packages" EXCLUDE)
endif ()

install(${CMAKE_CURRENT_SOURCE_DIR}/LICENSE DESTINATION ".")
install(${CMAKE_CURRENT_SOURCE_DIR}/README.md DESTINATION ".")

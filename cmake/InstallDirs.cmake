install(DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "${CMAKE_CURRENT_SOURCE_DIR}/shapes"
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        DESTINATION "."
        PATTERN "*.kra" EXCLUDE
        PATTERN "*.png" EXCLUDE)

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

install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        DESTINATION ".")

if (APPLE)
    install(FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/logo/logo-icon.icns
            DESTINATION "."
            RENAME "TemporaryEscape.icns")

    # Copy the Vulkan and MotelVK library to the install directory.
    # Without this the user won't be able to run the app.
    # Vulkan library is not distributed in the MacOS, only in the SDK!
    file(GLOB LIB_VULKAN_DYLIBS "$ENV{VULKAN_SDK}/lib/libvulkan.1*.dylib")
    install(FILES
            "$ENV{VULKAN_SDK}/lib/libMoltenVK.dylib"
            ${LIB_VULKAN_DYLIBS}
            DESTINATION ".")

    # We also need the MotelVK ICD JSON file.
    install(FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos/MoltenVK_icd.json
            DESTINATION "./vulkan/icd.d/")
elseif (UNIX)
    install(FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/logo/logo-icon.png
            DESTINATION "."
            RENAME "temporary-escape.png")
    install(FILES
            ${CMAKE_CURRENT_LIST_DIR}/linux/temporary-escape.desktop
            DESTINATION ".")
endif ()

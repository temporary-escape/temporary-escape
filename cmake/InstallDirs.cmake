install(DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        DESTINATION "."
        PATTERN "*.kra" EXCLUDE
        PATTERN "*.png" EXCLUDE
        PATTERN "*.h" EXCLUDE
        PATTERN "*.spirv" EXCLUDE
        PATTERN "*.glsl" EXCLUDE)

install(DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        DESTINATION ".")

if (MSVC)
    install(DIRECTORY
            "${CMAKE_BINARY_DIR}/"
            DESTINATION "."
            FILES_MATCHING PATTERN "*.dll"
            PATTERN ".cmake" EXCLUDE
            PATTERN "src" EXCLUDE
            PATTERN "test" EXCLUDE
            PATTERN "vcpkg_installed" EXCLUDE
            PATTERN "python" EXCLUDE
            PATTERN "CMakeFiles" EXCLUDE
            PATTERN "install" EXCLUDE
            PATTERN "release" EXCLUDE
            PATTERN "third_party" EXCLUDE
            PATTERN "TemporaryEscapeData" EXCLUDE
            PATTERN "TemporaryEscapeShaders" EXCLUDE
            PATTERN "Testing" EXCLUDE
            PATTERN "_CPack_Packages" EXCLUDE
            PATTERN "ZIP" EXCLUDE)
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
    file(GLOB LIB_VULKAN_DYLIBS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos/vulkan-sdk/libvulkan*.dylib")
    install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos/vulkan-sdk/libMoltenVK.dylib"
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

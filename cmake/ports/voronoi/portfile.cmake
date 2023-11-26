vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO JCash/voronoi
        REF "v0.9.0"
        SHA512 6903da515ae3ca7deaada7e68ce6024e0211c6727151061abeb89b7d454ec0eb898399c7482f77ef1763d262b9fec612739a90dcefd54a92a9266b69d0df3bd3
        HEAD_REF master
        PATCHES
        cmake.patch
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_install_cmake()

file(
        INSTALL "${SOURCE_PATH}/LICENSE"
        DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
        RENAME copyright
)

file(
        COPY "${CMAKE_CURRENT_LIST_DIR}/usage"
        DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
)

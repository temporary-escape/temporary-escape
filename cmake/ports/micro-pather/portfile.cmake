vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO leethomason/MicroPather
        REF "33a3b8403f1bc3937c9d364fe6c3977169bee3b5"
        SHA512 fdb86b5ddd7e6a755bbc2530b7ad6ab353587262216865aaf68993f103f91cd39ec07dbd881464b9260e4ecb93ce8640ef4d7c96615e66ce028dd65831d6e036
        HEAD_REF master
        PATCHES
        cmake.patch
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_install_cmake()

file(
        INSTALL "${SOURCE_PATH}/LICENSE.txt"
        DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
        RENAME copyright
)

file(
        COPY "${CMAKE_CURRENT_LIST_DIR}/usage"
        DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
)

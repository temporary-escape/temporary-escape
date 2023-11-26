vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO Vavassor/ad_mipmap
        REF "6d3501deeb58b35a08f997ca565b340422d50ff4"
        SHA512 b844344eaeec270950930d5d4f81ec074fe7990af3baad801c6471d721a2fb6e69dafcb9eb565f66395661a45297ae4bbcaf5089009623d39db176bcb4c7fa0e
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

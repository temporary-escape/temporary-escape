vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO SRombauts/SimplexNoise
        REF "97e62c5b5e26c8edabdc29a6b0a277192be3746c"
        SHA512 b1e27d6630a902006636ec31c3b1cead1857b255edc8ae8db74cdfee25300337200be48264c6adc0e2c0e39a3693e361d448e61ae62d2f0b700374b264d5e5cd
        HEAD_REF master
        PATCHES
        cmake.patch
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}/cmake"
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

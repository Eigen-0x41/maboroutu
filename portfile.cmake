vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Eigen-0x41/maboroutu
    REF "${VERSION}" # 0.0.0.10
    SHA512 d472aeb61219dde0fbbd5eafc60055883ea7097ebb2f94c389713e9a1d0b39b01db8b6f2944d6e6f6988aadff091939ee1d2da9f0c3094fced035e1ace6d7ead
    HEAD_REF maboroutu
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

# vcpkg_cmake_config_fixup(PACKAGE_NAME "maboroutu")

# file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

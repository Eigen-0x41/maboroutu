vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Eigen-0x41/maboroutu
    REF "${VERSION}" # 0.0.1.7
    SHA512 e6f96ea602a2c4807a4b6ca3f49c26cb8bfcc71dc9e94ba7b2d842fad284b0f42d5626903c4787ae524b034bf2e8e7a0a5f5ddfa169c288119ef6a65c87486be
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

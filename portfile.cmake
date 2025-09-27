vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Eigen-0x41/maboroutu
    REF "${VERSION}" # 0.0.0.15
    SHA512 3eb1b089e64514aa8a5b4e4ff1cd1e85703081a27f25655e950aa49784780f608d4c43f24be764392aeea813ac692b69bc998d3c4d31a3cb105c2f05d26c54f6
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

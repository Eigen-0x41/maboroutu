vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Eigen-0x41/maboroutu
    REF "${VERSION}" # 0.0.0.19
    SHA512 a0e0db2f0718b73b663af4824dc3d514ff660697e48ad3e332070ad114f3c9f819591cf99764ab177c5b6151d4f9a257334d2d7c6d6af43fb8e916596f79ee3a
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

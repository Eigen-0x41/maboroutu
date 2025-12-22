vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Eigen-0x41/maboroutu
    REF "${VERSION}" # 0.0.1.4
    SHA512 5fc8ba82b08688f3b738a533cf9079dd920af3e366700f0ca0f5460eac17207ca06798350113da9a302f299e73a832c8a8a9246348ead335376e747420d0ec3d
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

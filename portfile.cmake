vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Eigen-0x41/${PROJECT_NAME}
    REF "${VERSION}"
    SHA512 92ea2fbaba52d4abff8e447e3ac65730d59bc2c3c6bbb7c170f628bbd7fa7ed7eee79dd51275b312605673b74b45b0db8980459c6198ff3b0d8e924337cc90ea
    HEAD_REF ${PROJECT_NAME}
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME ${PROJECT_NAME})

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

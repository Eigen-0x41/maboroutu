vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Eigen-0x41/maboroutu
    REF "${VERSION}" # 0.0.0.4
    SHA512 0
    HEAD_REF maboroutu
)

install(DIRECTORY "${SOURCE_PATH}/include/maboroutu"  DESTINATION "${CURRENT_PACKAGES_DIR}/include/maboroutu")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

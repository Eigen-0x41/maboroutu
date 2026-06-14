cmake_minimum_required(VERSION 3.28)

option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compile_commands.json." ON)

#option(ENABLE_FOO_TESTING "Enable tests" OFF)
#if(ENABLE_FOO_TESTING)
#  list(APPEND VCPKG_MANIFEST_FEATURES "FOO-testing")
#endif()
if(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Use the vcpkg package manager")
endif()

#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE Release)
#endif()

project(maboroutu
    VERSION 0.0.2.15
    DESCRIPTION "Eigen's personal C++ utility library."
    HOMEPAGE_URL ""
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(${PROJECT_NAME}_SHARED_LIBS "Build ${PROJECT_NAME} as a shared library." OFF)

if(${PROJECT_NAME}_SHARED_LIBS)
  add_library(${PROJECT_NAME} SHARED)
else()
  add_library(${PROJECT_NAME} STATIC)
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

file(GLOB_RECURSE ${PROJECT_NAME}_MODULE_INTERFACE
  "${PROJECT_SOURCE_DIR}/src/*.cppm")
file(GLOB_RECURSE ${PROJECT_NAME}_MODULE_IMPLEMENTATION
  "${PROJECT_SOURCE_DIR}/src/*.cpp")
target_sources(${PROJECT_NAME}
  PUBLIC
  FILE_SET ${PROJECT_NAME}_FILE_SET
  TYPE CXX_MODULES
  BASE_DIRS ${PROJECT_SOURCE_DIR}/src
  FILES ${${PROJECT_NAME}_MODULE_INTERFACE})
target_sources(${PROJECT_NAME}
  PRIVATE
  ${${PROJECT_NAME}_MODULE_IMPLEMENTATION})

### Dependencies
# find_package(${LIB} CONFIG REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})

### Transparency
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_PUBLIC)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_PRIVATE)

### install library select
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
  EXPORT
    ${PROJECT_NAME}-targets
  LIBRARY DESTINATION
    ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION
    ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}
  FILE_SET ${PROJECT_NAME}_FILE_SET DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
  CXX_MODULES_BMI DESTINATION "")

if(EXISTS $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>include/${PROJECT_NAME})
  install(DIRECTORY $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>include/${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

install(EXPORT ${PROJECT_NAME}-targets
  FILE ${PROJECT_NAME}-targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})
### package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)
file(WRITE
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.cmake.in
  "@PACKAGE_INIT@\n"
  "include(CMakeFindDependencyMacro)\n"
  # "find_dependency(${lib} CONFIG REQUIRED)\n"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-targets.cmake\")\n"
  "check_required_components(\"@PROJECT_NAME@\")")
configure_package_config_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})

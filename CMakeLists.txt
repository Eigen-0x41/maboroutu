cmake_minimum_required(VERSION 3.28)

option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compile_commands.json." OFF)

option(ENABLE_MABOROUTU_TESTING "Enable tests" OFF)
if(ENABLE_MABOROUTU_TESTING)
  list(APPEND VCPKG_MANIFEST_FEATURES "maboroutu-testing")
endif()
if(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Use the vcpkg package manager")
endif()

#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE Release)
#endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # clang++
  add_compile_options($<IF:$<CONFIG:Debug>,-g,-O2>
      -Wall
      -Wextra
      --pedantic-errors
      -fno-rtti)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # g++
  add_compile_options($<IF:$<CONFIG:Debug>,-g,-O2>
      -Wall
      -Wextra
      --pedantic-errors
      -fno-rtti)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # icc, icx
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # cl, clang-cl
  add_compile_options($<IF:$<CONFIG:Debug>,/Z7,/O2>
      /EHsc
      /W4
      /permissive-
      /GR-)
endif()


project(maboroutu
    VERSION 0.0.1.8
    DESCRIPTION "Eigen's personal C++ utility library."
    HOMEPAGE_URL ""
    LANGUAGES CXX)


include(GNUInstallDirs)

### Dependencies
# find_package(${LIB} CONFIG REQUIRED)

### Main
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
# target_compile_options(${PROJECT_NAME} PRIVATE -pthread)

### link Dependencies
# target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})

### Transparency
target_compile_definitions(${PROJECT_NAME} INTERFACE ${PROJECT_NAME}_INTERFACE)

### install library select
# set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "lib1.h")

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Development
  BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime)
install(EXPORT ${PROJECT_NAME}Targets
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME})
install(DIRECTORY $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/CMakeConfig.in.txt"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})
write_basic_package_version_file(
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION 0.1
  COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})


### Test Dependencies
if(ENABLE_MABOROUTU_TESTING)
  enable_testing()

  find_package(GTest CONFIG REQUIRED)
  add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endif()

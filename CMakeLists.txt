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
    LANGUAGES CXX
)

include(GNUInstallDirs)

### Dependencies
# find_package(${LIB} CONFIG REQUIRED)

### Main
add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
# target_compile_options(${PROJECT_NAME} PRIVATE -pthread)

### link Dependencies
# target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})

### Transparency
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_PUBLIC)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_PRIVATE)

### install library select
# set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "lib1.h")
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>include/${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ${PROJECT_NAME}-targets
  FILE ${PROJECT_NAME}-targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})

file(WRITE
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.cmake.in
  "@PACKAGE_INIT@\n"
  "include(CMakeFindDependencyMacro)\n"
  # "find_dependency(${lib} CONFIG REQUIRED)\n"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}-targets.cmake\")\n"
  "check_required_components(\"@PROJECT_NAME@\")")
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})


# ### Test Dependencies
# # find_package(Boost REQUIRED COMPONENTS test)
# find_package(Boost REQUIRED COMPONENTS unit_test_framework)
# add_subdirectory(${PROJECT_SOURCE_DIR}/test)

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

project(maboroutu-vcpkg
    VERSION 0.0.0.0
    DESCRIPTION "TODO: ADD DESCRIPTION"
    HOMEPAGE_URL ""
    LANGUAGES CXX)

### Project Dependencies
find_package(maboroutu REQUIRED)

### Main
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
#target_compile_options(${PROJECT_NAME} PRIVATE -pthread)

### Dependencies
# target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})
target_link_libraries(${PROJECT_NAME} PRIVATE maboroutu::maboroutu)

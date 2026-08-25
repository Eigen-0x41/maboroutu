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

project(maboroutu LANGUAGES NONE)
set(WORKSPACE_NAME "${CMAKE_PROJECT_NAME}"
  CACHE STRING "Name of the overall workspace")

add_subdirectory(core)
option(MABOROUTU_ENABLE_EXTENSION
   "Build maboroutu_extension modules (slot_map, keyed_slot_map, stream_data_source). Set to OFF to disable."
   ON)
if(MABOROUTU_ENABLE_EXTENSION)
  add_subdirectory(extension)
endif()

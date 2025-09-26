cmake_minimum_required(VERSION 3.15...3.26)

# clang-tidy
if(NOT CMAKE_EXPORT_COMPILE_COMMANDS EQUAL ON)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE Release)
#endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # clang++
  add_compile_options(-Wall -Wextra --pedantic-errors)
  add_compile_options($<IF:$<CONFIG:Debug>,-g,-O2>)
  # add_compile_options($<IF:$<CONFIG:Debug>,-g,-O3>)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # g++
  add_compile_options(-Wall -Wextra --pedantic-errors)
  add_compile_options($<IF:$<CONFIG:Debug>,-g,-O2>)
  # add_compile_options($<IF:$<CONFIG:Debug>,-g,-O3>)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # icc, icx
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # cl, clang-cl
  add_compile_options(/EHsc /W4 /permissive-)
  add_compile_options($<IF:$<CONFIG:Debug>,/Z7,/O2>)
  # add_compile_options($<IF:$<CONFIG:Debug>,/Z7,/Ot>)
endif()

project(maboroutu
    VERSION 0.0.0.3
    DESCRIPTION "Eigen's personal C++ utility library."
    HOMEPAGE_URL ""
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(TargetName maboroutu)

### Dependencies
# find_package(${LIB} CONFIG REQUIRED)

### Main
add_library(${TargetName} STATIC ${PROJECT_SOURCE_DIR}/src/${TargetName}.cpp)
target_include_directories(${TargetName} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(${TargetName} PUBLIC cxx_std_23)
# target_compile_options(${TargetName} PRIVATE -pthread)

### link Dependencies
# target_link_libraries(${TargetName} PRIVATE ${LIB})

### Transparency
target_compile_definitions(${TargetName} PUBLIC ${TargetName}_PUBLIC)
target_compile_definitions(${TargetName} PRIVATE ${TargetName}_PRIVATE)

### install library select
# set_target_properties(${TargetName} PROPERTIES PUBLIC_HEADER "lib1.h")
install(TARGETS ${TargetName} EXPORT ${TargetName}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  # ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  # RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  # PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include
  DESTINATION include
)
install(EXPORT ${TargetName}Targets
  FILE ${TargetName}Targets.cmake
  NAMESPACE ${TargetName}::
  DESTINATION share/${TargetName}
)
file(WRITE
  "${CMAKE_CURRENT_BINARY_DIR}/${TargetName}Config.cmake"
  "include(CMakeFindDependencyMacro)\n"
  # "find_dependency(${lib} CONFIG REQUIRED)\n"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/${TargetName}Targets.cmake\")\n"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${TargetName}Config.cmake"
  DESTINATION share/${TargetName}
)

# enable_testing()
# ### Test Dependencies
# # find_package(Boost REQUIRED COMPONENTS test)
# find_package(Boost REQUIRED COMPONENTS unit_test_framework)
# add_subdirectory(test)

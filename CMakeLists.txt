cmake_minimum_required(VERSION 3.15...3.26)

# clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE Release)
#endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # clang++
  add_compile_options(-Wall -Wextra --pedantic-errors)
  add_compile_options($<IF:$<CONFIG:Debug>, -g, -O2>)
  # add_compile_options($<IF:$<CONFIG:Debug>, -g, -O3>)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # g++
  add_compile_options(-Wall -Wextra --pedantic-errors)
  add_compile_options($<IF:$<CONFIG:Debug>, -g, -O2>)
  # add_compile_options($<IF:$<CONFIG:Debug>, -g, -O3>)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # icc, icx
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # cl, clang-cl
  add_compile_options(/EHsc /W4 /permissive-)
  add_compile_options($<IF:$<CONFIG:Debug>, /Z7, /O2>)
  # add_compile_options($<IF:$<CONFIG:Debug>, /Z7, /Ot>)
endif()


project(maboroutu
    VERSION 0.0.0.0
    DESCRIPTION "Eigen's personal C++ utility library."
    HOMEPAGE_URL ""
    LANGUAGES CXX)

### Project Dependencies
# add_subdirectory(${PROJECT_SOURCE_DIR}/lib/${DEPEND})

### Project Dependencies

### Main
add_library(${PROJECT_NAME} STATIC
  ${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}.cpp
  ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}.hpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
# target_compile_options(${PROJECT_NAME} PRIVATE -pthread)

### Dependencies
# target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})

### Transparency
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_PUBLIC)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_PRIVATE)


# enable_testing()
# ### Test Dependencies
# # find_package(Boost REQUIRED COMPONENTS test)
# find_package(Boost REQUIRED COMPONENTS unit_test_framework)
# add_subdirectory(test)

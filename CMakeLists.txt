cmake_minimum_required(VERSION 3.15...3.26)

# clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS=ON)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE Release)
#endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra --pedantic-errors")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

### Main
project(maboroutu
    VERSION 0.0.0.0
    DESCRIPTION "Eigen's personal C++ utility library."
    HOMEPAGE_URL ""
    LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC
  ${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}.cpp
  ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}.hpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
# target_compile_options(${PROJECT_NAME} PRIVATE -pthread)

### Dependencies
# target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})
# target_include_directories(${PROJECT_NAME} PRIVATE ${LIB})

### Transparency
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_PUBLIC)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_PRIVATE)


### Test
project(maboroutu_test
    VERSION 0.0.0.0
    DESCRIPTION "maboroutu library test executable project."
    HOMEPAGE_URL ""
    LANGUAGES CXX)

### Project Dependencies
# add_subdirectory(${PROJECT_SOURCE_DIR}/lib/${DEPEND})

### Main
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

### Execute Dependencies
# target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})

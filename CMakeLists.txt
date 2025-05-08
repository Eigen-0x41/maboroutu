cmake_minimum_required(VERSION 3.25...3.26)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
# Default to C++ extensions being off. Clang's modules support have trouble
# with extensions right now.
set(CMAKE_CXX_EXTENSIONS OFF)

project(maboroutu
    VERSION 0.0.0.0
    DESCRIPTION "Eigen's personal C++ utility library."
    HOMEPAGE_URL ""
    LANGUAGES CXX)

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
# target_include_directories(${PROJECT_NAME} PRIVATE ${LIB})

### Transparency
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_PUBLIC)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_PRIVATE)

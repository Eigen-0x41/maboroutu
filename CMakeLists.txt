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
    VERSION 0.0.0.0
    DESCRIPTION "TODO: ADD DESCRIPTION"
    HOMEPAGE_URL ""
    LANGUAGES CXX
)

set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

### Project Dependencies

### Main
add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}.hpp)
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
# target_compile_options(${PROJECT_NAME} PRIVATE -pthread)

### Dependencies
# target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})

### Transparency
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_PUBLIC)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_PRIVATE)

### select install
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Export
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
#  PUBLIC_HEADER DESTINATION ${PROJECT_SOURCE_DIR}/include
)

### create ${PROJECT_NAME}Config.cmake
install(EXPORT ${PROJECT_NAME}Export
  FILE ${PROJECT_NAME}Config.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION lib/cmake/${PROJECT_NAME}/
)

# enable_testing()
# ### Test Dependencies
# # find_package(Boost REQUIRED COMPONENTS test)
# find_package(Boost REQUIRED COMPONENTS unit_test_framework)
# add_subdirectory(test)

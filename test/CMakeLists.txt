### Test: UniqueFile
### Test Main
add_executable(${PROJECT_NAME}_UniqueFileTest ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/UniqueFile.cpp)
# target_include_directories(${PROJECT_NAME}_UniqueFileTest PUBLIC ${PROJECT_SOURCE_DIR}/include)
#target_compile_options(${PROJECT_NAME}_UniqueFileTest PUBLIC -Wno-dev)

### Test Execute Dependencies
# target_link_libraries(${PROJECT_NAME}_UniqueFileTest PRIVATE Boost::test)
target_link_libraries(${PROJECT_NAME}_UniqueFileTest PRIVATE Boost::unit_test_framework)
target_link_libraries(${PROJECT_NAME}_UniqueFileTest PRIVATE maboroutu)

add_test(NAME ${PROJECT_NAME}_UniqueFile COMMAND ${PROJECT_NAME}_UniqueFileTest)


### Test: SlotMap
### Test Main
add_executable(${PROJECT_NAME}_SlotMapTest ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/SlotMap.cpp)
# target_include_directories(${PROJECT_NAME}_SlotMapTest PUBLIC ${PROJECT_SOURCE_DIR}/include)
#target_compile_options(${PROJECT_NAME}_SlotMapTest PUBLIC -Wno-dev)

### Test Execute Dependencies
# target_link_libraries(${PROJECT_NAME}_SlotMapTest PRIVATE Boost::test)
target_link_libraries(${PROJECT_NAME}_SlotMapTest PRIVATE Boost::unit_test_framework)
target_link_libraries(${PROJECT_NAME}_SlotMapTest PRIVATE maboroutu)

add_test(NAME ${PROJECT_NAME}_SlotMap COMMAND ${PROJECT_NAME}_SlotMapTest)


### Test: ExMemory
### Test Main
add_executable(${PROJECT_NAME}_ExMemoryTest ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/ExMemory.cpp)
# target_include_directories(${PROJECT_NAME}_ExMemoryTest PUBLIC ${PROJECT_SOURCE_DIR}/include)
# target_compile_options(${PROJECT_NAME}_ExMemoryTest PUBLIC -Wno-dev)

### Test Execute Dependencies
# target_link_libraries(${PROJECT_NAME}_ExMemoryTest PRIVATE Boost::test)
target_link_libraries(${PROJECT_NAME}_ExMemoryTest PRIVATE Boost::unit_test_framework)
target_link_libraries(${PROJECT_NAME}_ExMemoryTest PRIVATE maboroutu)

add_test(NAME ${PROJECT_NAME}_ExMemory COMMAND ${PROJECT_NAME}_ExMemoryTest)

### Test: BitAccess
### Test Main
add_executable(${PROJECT_NAME}_BitAccessTest ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/BitAccess.cpp)
# target_include_directories(${PROJECT_NAME}_BitAccessTest PUBLIC ${PROJECT_SOURCE_DIR}/include)
# target_compile_options(${PROJECT_NAME}_BitAccessTest PUBLIC -Wno-dev)

### Test Execute Dependencies
# target_link_libraries(${PROJECT_NAME}_BitAccessTest PRIVATE Boost::test)
target_link_libraries(${PROJECT_NAME}_BitAccessTest PRIVATE Boost::unit_test_framework)
target_link_libraries(${PROJECT_NAME}_BitAccessTest PRIVATE maboroutu)

add_test(NAME ${PROJECT_NAME}_BitAccess COMMAND ${PROJECT_NAME}_BitAccessTest)

### Test: Error
### Test Main
add_executable(${PROJECT_NAME}_ErrorTest ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/Error.cpp)
# target_include_directories(${PROJECT_NAME}_ErrorTest PUBLIC ${PROJECT_SOURCE_DIR}/include)
# target_compile_options(${PROJECT_NAME}_ErrorTest PUBLIC -Wno-dev)

### Test Execute Dependencies
# target_link_libraries(${PROJECT_NAME}_ErrorTest PRIVATE Boost::test)
target_link_libraries(${PROJECT_NAME}_ErrorTest PRIVATE Boost::unit_test_framework)
target_link_libraries(${PROJECT_NAME}_ErrorTest PRIVATE maboroutu)

add_test(NAME ${PROJECT_NAME}_Error COMMAND ${PROJECT_NAME}_ErrorTest)

### Test: LinkNode
### Test Main
add_executable(${PROJECT_NAME}_LinkNodeTest ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/LinkNode.cpp)
# target_include_directories(${PROJECT_NAME}_LinkNodeTest PUBLIC ${PROJECT_SOURCE_DIR}/include)
# target_compile_options(${PROJECT_NAME}_LinkNodeTest PUBLIC -Wno-dev)

### Test Execute Dependencies
# target_link_libraries(${PROJECT_NAME}_LinkNodeTest PRIVATE Boost::test)
target_link_libraries(${PROJECT_NAME}_LinkNodeTest PRIVATE Boost::unit_test_framework)
target_link_libraries(${PROJECT_NAME}_LinkNodeTest PRIVATE maboroutu)

add_test(NAME ${PROJECT_NAME}_LinkNode COMMAND ${PROJECT_NAME}_LinkNodeTest)

### Test: BinaryBuffer
### Test Main
add_executable(${PROJECT_NAME}_BinaryBuffer ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/BinaryBuffer.cpp)
# target_compile_features(${DEPEND_PROJECT_NAME}_BinaryBuffer PRIVATE cxx_std_23)
# target_include_directories(${DEPEND_PROJECT_NAME}_BinaryBuffer PUBLIC ${PROJECT_SOURCE_DIR}/include)
# target_compile_options(${DEPEND_PROJECT_NAME}_BinaryBuffer PUBLIC -Wno-dev)

### Test Execute Dependencies
# target_link_libraries(${DEPEND_PROJECT_NAME}_BinaryBuffer PRIVATE Boost::test)
target_link_libraries(${PROJECT_NAME}_BinaryBuffer PRIVATE Boost::unit_test_framework)
target_link_libraries(${PROJECT_NAME}_BinaryBuffer PRIVATE maboroutu)

add_test(NAME ${PROJECT_NAME}_BinaryBuffer COMMAND ${PROJECT_NAME}_BinaryBuffer)

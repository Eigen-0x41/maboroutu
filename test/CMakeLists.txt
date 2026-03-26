# ### Test: UniqueFile
# ### Test Main
# add_executable(${PROJECT_NAME}_UniqueFileTest ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/UniqueFile.cpp)
# # target_include_directories(${PROJECT_NAME}_UniqueFileTest PUBLIC ${PROJECT_SOURCE_DIR}/include)
# #target_compile_options(${PROJECT_NAME}_UniqueFileTest PUBLIC -Wno-dev)
#
# ### Test Execute Dependencies
# # target_link_libraries(${PROJECT_NAME}_UniqueFileTest PRIVATE Boost::test)
# target_link_libraries(${PROJECT_NAME}_UniqueFileTest PRIVATE Boost::unit_test_framework)
# target_link_libraries(${PROJECT_NAME}_UniqueFileTest PRIVATE maboroutu)
#
# add_test(NAME ${PROJECT_NAME}_UniqueFile COMMAND ${PROJECT_NAME}_UniqueFileTest)
#
#
# ### Test: SlotMap
# ### Test Main
# add_executable(${PROJECT_NAME}_SlotMapTest ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/SlotMap.cpp)
# # target_include_directories(${PROJECT_NAME}_SlotMapTest PUBLIC ${PROJECT_SOURCE_DIR}/include)
# #target_compile_options(${PROJECT_NAME}_SlotMapTest PUBLIC -Wno-dev)
#
# ### Test Execute Dependencies
# # target_link_libraries(${PROJECT_NAME}_SlotMapTest PRIVATE Boost::test)
# target_link_libraries(${PROJECT_NAME}_SlotMapTest PRIVATE Boost::unit_test_framework)
# target_link_libraries(${PROJECT_NAME}_SlotMapTest PRIVATE maboroutu)
#
# add_test(NAME ${PROJECT_NAME}_SlotMap COMMAND ${PROJECT_NAME}_SlotMapTest)
#
# ### Test: BitAccess
# ### Test Main
# add_executable(${PROJECT_NAME}_BitAccessTest ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/BitAccess.cpp)
# # target_include_directories(${PROJECT_NAME}_BitAccessTest PUBLIC ${PROJECT_SOURCE_DIR}/include)
# # target_compile_options(${PROJECT_NAME}_BitAccessTest PUBLIC -Wno-dev)
#
# ### Test Execute Dependencies
# # target_link_libraries(${PROJECT_NAME}_BitAccessTest PRIVATE Boost::test)
# target_link_libraries(${PROJECT_NAME}_BitAccessTest PRIVATE Boost::unit_test_framework)
# target_link_libraries(${PROJECT_NAME}_BitAccessTest PRIVATE maboroutu)
#
# add_test(NAME ${PROJECT_NAME}_BitAccess COMMAND ${PROJECT_NAME}_BitAccessTest)


### Test: Error
### Test Main
add_executable(${PROJECT_NAME}_Error_sanitize_address_undef ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/Error.cpp)
# target_compile_features(${PROJECT_NAME}_Error_sanitize_address_undef PRIVATE cxx_std_23)
target_include_directories(${PROJECT_NAME}_Error_sanitize_address_undef PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_options(${PROJECT_NAME}_Error_sanitize_address_undef PRIVATE
  $<$<CONFIG:Release>:$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address>>
  $<$<CONFIG:Release>:$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=undefined>>
  $<$<CONFIG:Release>:$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fno-omit-frame-pointer>>
  $<$<CONFIG:Release>:$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-static>>)
target_link_options(${PROJECT_NAME}_Error_sanitize_address_undef PRIVATE
  $<$<CONFIG:Release>:$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=address>>
  $<$<CONFIG:Release>:$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fsanitize=undefined>>)
### Test Execute Dependencies
# target_link_libraries(${PROJECT_NAME}_Error_sanitize_address_undef PRIVATE Boost::test)
target_link_libraries(${PROJECT_NAME}_Error_sanitize_address_undef PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_link_libraries(${PROJECT_NAME}_Error_sanitize_address_undef PRIVATE maboroutu)
# target_link_libraries(${PROJECT_NAME}_Error_sanitize_address_undef PRIVATE ${PROJECT_NAME})
add_test(NAME ${PROJECT_NAME}_Error_sanitize_address_undef COMMAND ${PROJECT_NAME}_Error_sanitize_address_undef)

if (CMAKE_STREQUAL:SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "x86_64")
  ### Test Main
  add_executable(${PROJECT_NAME}_Error_sanitize_thread_memory ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/Error.cpp)
  # target_compile_features(${PROJECT_NAME}_Error_sanitize_thread_memory PRIVATE cxx_std_23)
  target_include_directories(${PROJECT_NAME}_Error_sanitize_thread_memory PUBLIC ${PROJECT_SOURCE_DIR}/include)
  target_compile_options(${PROJECT_NAME}_Error_sanitize_thread_memory PUBLIC
        $<$<CONFIG:Release>:-fsanitize=thread>
        $<$<CONFIG:Release>:-fsanitize=memory>
        $<$<CONFIG:Release>:-fno-omit-frame-pointer>
        $<$<CONFIG:Release>:-fPIE>
        $<$<CONFIG:Release>:-pie>)
  ### Test Execute Dependencies
  # target_link_libraries(${PROJECT_NAME}_Error_sanitize_thread_memory PRIVATE Boost::test)
  target_link_libraries(${PROJECT_NAME}_Error_sanitize_thread_memory PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
  target_link_libraries(${PROJECT_NAME}_Error_sanitize_thread_memory PRIVATE maboroutu)
  # target_link_libraries(${PROJECT_NAME}_Error_sanitize_thread_memory PRIVATE ${PROJECT_NAME})
  add_test(NAME ${PROJECT_NAME}_Error_sanitize_thread_memory COMMAND ${PROJECT_NAME}_Error_sanitize_thread_memory)
endif()


# ### Test: ReferenceCounter
# ### Test Main
# add_executable(${PROJECT_NAME}_ReferenceCounter ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/ReferenceCounter.cpp)
# # target_compile_features(${PROJECT_NAME}_ReferenceCounter PRIVATE cxx_std_23)
# # target_include_directories(${PROJECT_NAME}_ReferenceCounter PUBLIC ${PROJECT_SOURCE_DIR}/include)
# # target_compile_options(${PROJECT_NAME}_ReferenceCounter PUBLIC -Wno-dev)
#
# ### Test Execute Dependencies
# # target_link_libraries(${PROJECT_NAME}_ReferenceCounter PRIVATE Boost::test)
# target_link_libraries(${PROJECT_NAME}_ReferenceCounter PRIVATE Boost::unit_test_framework)
# target_link_libraries(${PROJECT_NAME}_ReferenceCounter PRIVATE ${PROJECT_NAME})
#
# add_test(NAME ${PROJECT_NAME}_ReferenceCounter COMMAND ${PROJECT_NAME}_ReferenceCounter)

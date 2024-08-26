add_subdirectory(catch2)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(
  TEST_SRCS
  src/tree++_test.cc
  src/test/treexx/bin/avl/index_tree_core_test.cc
  src/test/treexx/bin/avl/offset_tree_core_test.cc
  src/test/treexx/bin/avl/simple_tree_core_test.cc)

add_executable(
  tree++_test
  ${TEST_SRCS})

set_property(
  TARGET tree++_test
  APPEND
  PROPERTY
  COMPILE_DEFINITIONS $<$<CONFIG:Debug>:TREEXX_DEBUG=1>)

target_link_libraries(
  tree++_test
  treexx
  catch2)

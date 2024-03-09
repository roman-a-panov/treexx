add_subdirectory(catch2)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(
  tree++_test
  src/tree++_test.cc
  src/treexx/avl/tree_test.cc
  src/treexx/avl/simple_tree_core_test.cc)

target_link_libraries(
  tree++_test
  catch2)

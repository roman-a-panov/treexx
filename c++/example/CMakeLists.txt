include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(
  TARGETS
  index_avl_tree
  offset_avl_tree
  simple_avl_tree)

add_executable(
  simple_avl_tree
  src/simple_avl_tree.cc)

add_executable(
  index_avl_tree
  src/index_avl_tree.cc)

add_executable(
  offset_avl_tree
  src/offset_avl_tree.cc)

foreach(TARGET IN LISTS TARGETS)
  set_property(
    TARGET ${TARGET}
    APPEND
    PROPERTY
    COMPILE_DEFINITIONS $<$<CONFIG:Debug>:TREEXX_DEBUG=1>)
endforeach()

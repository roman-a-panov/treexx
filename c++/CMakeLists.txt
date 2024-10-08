set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  set(COMPILER_FLAGS "/bigobj")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")

  set(LINKER_FLAGS_RELEASE "/LTCG")
  set(
    CMAKE_EXE_LINKER_FLAGS_RELEASE
    "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${LINKER_FLAGS_RELEASE}")
  set(
    CMAKE_EXE_LINKER_FLAGS_MINSIZEREL
    "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} ${LINKER_FLAGS_RELEASE}")
  set(
    CMAKE_SHARED_LINKER_FLAGS_RELEASE
    "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${LINKER_FLAGS_RELEASE}")
  set(
    CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL
    "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL} ${LINKER_FLAGS_RELEASE}")
else()
  set(FLAGS "-Wall -Wunused-parameter")
  set(FLAGS_DEBUG "-g -O0 -fsanitize=address -fsanitize=undefined")
  set(FLAGS_RELEASE "-O3")

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")

  set(CMAKE_C_FLAGS_DEBUG "${FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS_DEBUG "${FLAGS_DEBUG}")

  set(CMAKE_C_FLAGS_RELEASE "${FLAGS_RELEASE}")
  set(CMAKE_CXX_FLAGS_RELEASE "${FLAGS_RELEASE}")
endif()

if(NOT "Debug" STREQUAL "${CMAKE_BUILD_TYPE}")
  set_property(
    DIRECTORY
    APPEND
    PROPERTY
    COMPILE_DEFINITIONS NDEBUG)
endif()

add_subdirectory(example)
add_subdirectory(main)
add_subdirectory(test)

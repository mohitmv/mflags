cmake_minimum_required(VERSION 3.1)

project(
  mflags
  VERSION 1.0
  LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)

add_library(mflags STATIC mflags.cpp mflags.h)

add_executable(mflags_test1 tests/mflags_test1.cpp)
target_include_directories(mflags_test1 PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(mflags_test1 PRIVATE mflags)

add_executable(mflags_test2 tests/mflags_test2.cpp)
target_include_directories(mflags_test2 PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(mflags_test2 PRIVATE mflags)

add_executable(mflags_example tests/mflags_example.cpp)
target_include_directories(mflags_example PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(mflags_example PRIVATE mflags)


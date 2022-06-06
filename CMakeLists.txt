cmake_minimum_required(VERSION 3.1)

project(
  mflags
  VERSION 1.0
  LANGUAGES CXX)

add_library(mflags STATIC mflags.cpp mflags.h)
target_compile_options(mflags PRIVATE -std=c++17)

if (${BUILD_MFLAGS_TESTS})
  add_executable(mflags_test1 tests/mflags_test1.cpp)
  target_include_directories(mflags_test1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(mflags_test1 PRIVATE mflags)
  target_compile_options(mflags_test1 PRIVATE -std=c++17)

  add_executable(mflags_test2 tests/mflags_test2.cpp)
  target_include_directories(mflags_test2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(mflags_test2 PRIVATE mflags)
  target_compile_options(mflags_test2 PRIVATE -std=c++17)

  add_executable(mflags_example tests/mflags_example.cpp)
  target_include_directories(mflags_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(mflags_example PRIVATE mflags)
  target_compile_options(mflags_example PRIVATE -std=c++17)
endif()

add_library(
  dependencies STATIC
  argh/argh.cpp)
target_include_directories(dependencies PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


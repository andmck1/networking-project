set(CMAKE_CXX_STANDARD 20)
cmake_minimum_required(VERSION 4.0.2)
project(NetworkingProject LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -gdwarf-4 -O0")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-Wl,-debug")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

# sfml
FetchContent_Declare(
  sfml
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 3.0.1
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL SYSTEM)

# spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG v1.15.3
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL SYSTEM)

# imgui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG v1.91.9
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL SYSTEM)

FetchContent_MakeAvailable(sfml spdlog imgui)

add_library(
  imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)

add_subdirectory(modules/dependencies)
add_subdirectory(modules/core)
add_subdirectory(modules/app)

add_library(
  core STATIC
  src/core_game/sprite/sprite.cpp
  src/core_game/texturemanager/texturemanager.cpp
  src/core_game/window/window.cpp)
target_link_libraries(core PRIVATE spdlog imgui sfml-graphics sfml-window
                                   sfml-system sfml-network)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(sfml PUBLIC ${SFML_SOURCE_DIR}/include)

target_include_directories(
  imgui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/imgui
               ${CMAKE_CURRENT_LIST_DIR}/imgui/backends)

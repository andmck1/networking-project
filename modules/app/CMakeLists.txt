include_directories(shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/shared)

set(SHARED_FILES
    src/shared/layer/app.cpp
    src/shared/layer/debug.cpp
    src/shared/actor/base.cpp
    src/shared/actor/ship.cpp
    src/shared/actor/projectile.cpp
    src/shared/config/settings.cpp
    src/shared/eventhandler/eventhandler.cpp
    src/shared/config/state.cpp
    src/shared/networking/messages.cpp
    src/shared/networking/clientconnection.cpp)

set(SERVER_FILES src/server/app.cpp ${SHARED_FILES}
                 src/server/networking/server.cpp)

set(CLIENT_FILES src/client/app.cpp src/client/networking/client.cpp src/client/utils/cli.cpp
                 ${SHARED_FILES})

# server
add_executable(server ${SERVER_FILES})
target_compile_features(server PRIVATE cxx_std_20)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}
                                        ${imgui_SOURCE_DIR}/backends)
target_link_libraries(
  server
  PRIVATE sfml-graphics
          sfml-network
          sfml-system
          sfml-window
          spdlog
          imgui
          core)

# client
add_executable(client ${CLIENT_FILES})
target_compile_features(client PRIVATE cxx_std_20)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}
                                        ${imgui_SOURCE_DIR}/backends)
target_link_libraries(
  client
  PRIVATE sfml-graphics
          sfml-network
          sfml-system
          sfml-window
          spdlog
          imgui
          dependencies
          core)

cmake_minimum_required(VERSION 3.10)
project(NetworkingProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


include(FetchContent)
FetchContent_Declare(sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(sfml)

FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.15.3
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(spdlog)

# server
add_executable(server src/server.cpp)
target_compile_features(server PRIVATE cxx_std_17)
target_link_libraries(server PRIVATE sfml-graphics sfml-network sfml-system sfml-window spdlog)

# client
add_executable(client src/client.cpp)
target_compile_features(client PRIVATE cxx_std_17)
target_link_libraries(client PRIVATE sfml-graphics sfml-network sfml-system sfml-window spdlog)


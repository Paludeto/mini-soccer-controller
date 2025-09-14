cmake_minimum_required(VERSION 3.10)

# Nome do projeto e versão
project(soccer-controller LANGUAGES CXX)

# Roda CMakeLists dos arquivos protobuf
add_subdirectory(proto)

# Encontra arquivos do Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Network)
find_package(SDL2 REQUIRED)

add_executable(soccer-controller main.cpp)

# Linka bibliotecas
target_link_libraries(soccer-controller
    Qt6::Core
    Qt6::Network
    SDL2::SDL2
    pb_msgs_lib
)
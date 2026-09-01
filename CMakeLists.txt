cmake_minimum_required(VERSION 3.25)
project(Synapse LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)

include(cmake/vendor.cmake)

add_executable(Synapse
  src/main.cpp
  src/websocket.cpp
)

target_include_directories(Synapse
  PRIVATE
    src
    include
)

target_link_libraries(Synapse
  PRIVATE
    spdlog::spdlog
    OpenSSL::SSL
    OpenSSL::Crypto
)

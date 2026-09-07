cmake_minimum_required(VERSION 3.25)
project(Synapse LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(vendor/vendor.cmake)

find_package(OpenSSL REQUIRED)

add_executable(Synapse
  src/main.cpp
  src/websocket.cpp
)

target_include_directories(Synapse
  PRIVATE
    src
    include
    vendor
)

target_link_libraries(Synapse
  PRIVATE
    spdlog
    OpenSSL::SSL
    OpenSSL::Crypto
)

cmake_minimum_required(VERSION 3.25)

# ---------------------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------------------

include(vendor/vendor.cmake)

project(Synapse VERSION 0.1 LANGUAGES CXX)

# ---------------------------------------------------------------------------------------
# Compiler config
# ---------------------------------------------------------------------------------------

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)

# ---------------------------------------------------------------------------------------
# Target config
# ---------------------------------------------------------------------------------------

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

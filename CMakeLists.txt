cmake_minimum_required(VERSION 3.25)

# ---------------------------------------------------------------------------------------
# Start xichedelic project
# ---------------------------------------------------------------------------------------

project(XICHEDELIC VERSION 0.1 LANGUAGES CXX)

# ---------------------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------------------

include(vendor/vendor.cmake)

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

set(XICHEDELIC_SOURCES
  src/websocket.cpp
  src/websocket_handshake.cpp
)

add_library(XICHEDELIC STATIC ${XICHEDELIC_SOURCES})

target_include_directories(XICHEDELIC
  PUBLIC
    include
  PRIVATE
    src
    vendor
)

target_link_libraries(XICHEDELIC
  PRIVATE
    spdlog
    OpenSSL::SSL
    OpenSSL::Crypto
)

add_executable(XICHEDELIC_TEST
  test/main.cpp
)

target_link_libraries(XICHEDELIC_TEST
  PRIVATE
    XICHEDELIC
)

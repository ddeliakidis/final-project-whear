cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(whear_app_core)

target_sources(app PRIVATE
    src/main.c
    src/ipc_app.c
    src/wifi_manager.c
    src/ws_client.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

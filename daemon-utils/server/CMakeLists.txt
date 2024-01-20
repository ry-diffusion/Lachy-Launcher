cmake_minimum_required (VERSION 2.6)

project(daemon-server-utils LANGUAGES CXX)

add_library(daemon-server-utils include/daemon_utils/auto_shutdown_service.h src/auto_shutdown_service.cpp)
target_include_directories(daemon-server-utils PUBLIC include/)
target_link_libraries(daemon-server-utils logger simpleipc file-util)
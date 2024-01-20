cmake_minimum_required (VERSION 2.6)

project(daemon-client-utils LANGUAGES CXX)

add_library(daemon-client-utils include/daemon_utils/daemon_launcher.h src/daemon_launcher.cpp include/daemon_utils/launchable_service_client.h)
target_include_directories(daemon-client-utils PUBLIC include/)
target_link_libraries(daemon-client-utils logger simpleipc file-util)
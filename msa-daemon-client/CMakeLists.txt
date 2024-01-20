cmake_minimum_required (VERSION 2.6)

project(msa-daemon-client LANGUAGES CXX)

add_library(msa-daemon-client include/msa/client/service_client.h include/msa/client/account.h include/msa/client/service_launcher.h src/service_client.cpp include/msa/client/security_scope.h include/msa/client/token.h src/token.cpp include/msa/client/legacy_token.h include/msa/client/compact_token.h)
target_include_directories(msa-daemon-client PUBLIC include/)
target_link_libraries(msa-daemon-client simpleipc logger base64 daemon-client-utils)
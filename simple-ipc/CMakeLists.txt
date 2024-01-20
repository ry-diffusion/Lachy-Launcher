cmake_minimum_required (VERSION 2.6)

project(simpleipc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

find_package(Threads)

add_library(simpleipc include/simpleipc/common/connection.h include/simpleipc/common/connection_handler.h include/simpleipc/server/rpc_handler.h include/simpleipc/common/rpc_call_exception.h include/simpleipc/common/rpc_result.h include/simpleipc/common/io_handler.h include/simpleipc/server/service.h include/simpleipc/server/service_impl.h include/simpleipc/client/service_client.h include/simpleipc/client/service_client_impl.h include/simpleipc/client/rpc_json_call.h include/simpleipc/common/message/message_id.h include/simpleipc/common/message/rpc_message.h include/simpleipc/common/message/response_message.h include/simpleipc/common/message/error_code.h include/simpleipc/common/message/error_message.h include/simpleipc/common/version.h src/common/connection_internal.cpp src/common/connection_internal.h src/client/base_service_client_impl.h src/server/base_service_impl.h src/server/default_rpc_handler.h src/server/default_rpc_handler.cpp src/unix/common/unix_connection.cpp src/unix/common/unix_connection.h src/unix/server/unix_service_impl.cpp src/unix/server/unix_service_impl.h src/unix/client/unix_service_client.cpp src/unix/client/unix_service_client.h src/common/encoding/encoding.h src/common/encoding/encodings.cpp src/common/encoding/encodings.h src/common/encoding/encoding_json.cpp src/common/encoding/encoding_json.h src/common/encoding/encoding_json_cbor.cpp src/common/encoding/encoding_json_cbor.h src/common/encoding/varint.cpp src/common/encoding/varint.h src/server/rpc_handler.cpp src/common/message/error_code.cpp src/common/message/message_container.h src/client/service_client.cpp src/client/rpc_json_call.cpp include/simpleipc/client/rpc_call.h)

if(APPLE)
  target_sources(simpleipc PRIVATE src/unix/kqueue_io_handler.cpp src/unix/kqueue_io_handler.h)
else()
  target_sources(simpleipc PRIVATE src/unix/epoll_io_handler.cpp src/unix/epoll_io_handler.h)
endif()

target_include_directories(simpleipc PUBLIC include/)
target_link_libraries(simpleipc Threads::Threads nlohmann_json)

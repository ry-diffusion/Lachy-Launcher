cmake_minimum_required (VERSION 2.6)

project(cll-telemetry LANGUAGES CXX)

include(CTest)

find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_library(cll-telemetry include/cll/event_manager.h src/event_manager.cpp include/cll/event.h include/cll/event_uploader.h include/cll/configuration.h src/configuration.cpp include/cll/configuration_cache.h include/cll/configuration_manager.h src/json_utils.h src/http/curl_request.cpp src/http/curl_request.h src/http/curl_client.cpp src/http/curl_client.h src/http/curl_error.h include/cll/http/http_response.h include/cll/file_configuration_cache.h src/file_configuration_cache.cpp include/cll/event_batch.h include/cll/file_event_batch.h src/file_event_batch.cpp include/cll/event_serializer.h src/event_serializer.cpp src/event_serializer_extensions.cpp include/cll/event_serializer_extensions.h include/cll/memory_event_batch.h src/memory_event_batch.cpp include/cll/multi_file_event_batch.h src/multi_file_event_batch.cpp include/cll/buffered_event_batch.h src/buffered_event_batch.cpp src/buffered_event_batch_private.h include/cll/task_with_delay_thread.h src/task_with_delay_thread.cpp src/event_uploader.cpp include/cll/event_upload_step.h include/cll/http/http_error.h include/cll/http/http_request.h include/cll/http/http_client.h include/cll/http/mock_http_client.h src/http/mock_http_client.cpp src/http/mock_http_request.h include/cll/event_compressor.h src/event_compressor.cpp)
target_include_directories(cll-telemetry PUBLIC include/)
target_include_directories(cll-telemetry PRIVATE ${CURL_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_link_libraries(cll-telemetry nlohmann_json logger Threads::Threads ${CURL_LIBRARIES} ${ZLIB_LIBRARIES})

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
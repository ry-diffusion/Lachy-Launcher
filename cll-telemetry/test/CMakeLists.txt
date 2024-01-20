find_package(GTest REQUIRED)

add_executable(cll-test main.cpp file_event_batch.cpp event_serializer.cpp memory_event_batch.cpp event_batch.cpp event_batch.h multi_file_event_batch.cpp buffered_event_batch.cpp file_configuration_cache.cpp task_with_delay_thread.cpp mock_http_client.cpp configuration.cpp event_uploader.cpp event_compressor.cpp)
target_include_directories(cll-test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(cll-test cll-telemetry ${GTEST_LIBRARIES})

add_test(cll-telemetry cll-test)
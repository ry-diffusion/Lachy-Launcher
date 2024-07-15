cmake_minimum_required (VERSION 3.10)
project (LachyCoreMods LANGUAGES CXX)


add_library(MoreInfos SHARED src/MoreInfos.cc)
target_link_libraries(MoreInfos coremod)

set_target_properties(MoreInfos PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../mcpelauncher-client/coremods)
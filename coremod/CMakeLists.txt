project (CoreMod LANGUAGES CXX)
cmake_minimum_required (VERSION 3.10)


add_library(coremod src/coremod.cpp)
target_link_libraries(coremod minecraft-symbols)
target_link_libraries(coremod gamewindow)
target_link_libraries(coremod logger)
target_include_directories(coremod PUBLIC include/)

cmake_minimum_required (VERSION 2.6)

project(filepicker LANGUAGES CXX)

set(FILEPICKER_USE_ZENITY true)
set(FILEPICKER_USE_COCOA false)

if (APPLE)
    set(FILEPICKER_USE_ZENITY false)
    set(FILEPICKER_USE_COCOA true)
endif()

set(FILEPICKER_SOURCES include/file_picker.h include/file_picker_factory.h src/file_picker_factory.cpp)
set(FILEPICKER_DEFINES )
if (FILEPICKER_USE_ZENITY)
    set(FILEPICKER_SOURCES ${FILEPICKER_SOURCES} src/file_picker_zenity.cpp src/file_picker_zenity.h)
    set(FILEPICKER_DEFINES ${FILEPICKER_DEFINES} FILEPICKER_USE_ZENITY)
endif()
if (FILEPICKER_USE_COCOA)
    set(FILEPICKER_SOURCES ${FILEPICKER_SOURCES} src/file_picker_cocoa.mm src/file_picker_cocoa.h)
    set(FILEPICKER_DEFINES ${FILEPICKER_DEFINES} FILEPICKER_USE_COCOA)
endif()

add_library(filepicker ${FILEPICKER_SOURCES})
target_include_directories(filepicker PUBLIC include/)
target_compile_definitions(filepicker PRIVATE ${FILEPICKER_DEFINES})
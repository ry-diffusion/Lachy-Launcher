cmake_minimum_required (VERSION 2.6)

project(eglut LANGUAGES C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

find_package(PNG REQUIRED)
find_package(EGL REQUIRED)

set(EGLUT_USE_X11 true)

set(EGLUT_SOURCES include/eglut.h src/eglut.c src/eglutint.h)
set(EGLUT_X11_SOURCES src/eglut_x11.c include/eglut_x11.h src/xinput.c src/xinput.h)

if (EGLUT_USE_X11)
    find_package(X11 REQUIRED)
    set(EGLUT_SOURCES ${EGLUT_SOURCES} ${EGLUT_X11_SOURCES})
endif()

add_library(eglut ${EGLUT_SOURCES})
target_include_directories(eglut PUBLIC include/)
target_include_directories(eglut PRIVATE ${EGL_INCLUDE_DIRS})
target_link_libraries(eglut PUBLIC ${PNG_LIBRARIES} ${EGL_LIBRARIES})

if (EGLUT_USE_X11)
    target_link_libraries(eglut PUBLIC ${X11_X11_LIB})
    target_include_directories(eglut PRIVATE ${X11_INCLUDE_DIR})
endif()
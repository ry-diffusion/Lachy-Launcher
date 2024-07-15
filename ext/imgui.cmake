include(FetchContent)

if (NOT DEFINED IMGUI_USE_GLFW)
    set (IMGUI_USE_GLFW false)
endif()

if (NOT DEFINED IMGUI_USE_EGLUT)
 set (IMGUI_USE_EGLUT false)
endif()

FetchContent_Declare(imgui_external
        URL https://github.com/ocornut/imgui/archive/refs/tags/v1.90.8.tar.gz
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(imgui_external)

set (IMGUI_SRCS
        ${imgui_external_SOURCE_DIR}/imgui.cpp
        ${imgui_external_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_external_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_external_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_external_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)

if (IMGUI_USE_GLFW)
 set (IMGUI_SRCS ${IMGUI_SRCS} ${imgui_external_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
endif()

if (IMGUI_USE_EGLUT)
    set (IMGUI_SRCS ${IMGUI_SRCS} ${imgui_external_SOURCE_DIR}/backends/imgui_impl_glut.cpp)
endif()

add_library(imgui ${IMGUI_SRCS})

target_include_directories(imgui PUBLIC ${imgui_external_SOURCE_DIR})

if (IMGUI_USE_GLFW)
    target_link_libraries(imgui PUBLIC glfw)
endif()
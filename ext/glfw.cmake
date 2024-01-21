set (GLFW_VERSION 3.3.8)
set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if (NOT glfw3_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    glfw3
    URL https://github.com/glfw/glfw/releases/download/${GLFW_VERSION}/glfw-${GLFW_VERSION}.zip
  )
  FetchContent_GetProperties(glfw3)

   if (NOT glfw3_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(glfw3)

    
    add_subdirectory(${glfw3_SOURCE_DIR} ${glfw3_BINARY_DIR})
  endif()
endif() 





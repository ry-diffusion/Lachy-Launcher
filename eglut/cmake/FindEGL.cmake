# Finds the `egl` library.
# This file is released under the Public Domain.
# Once done this will define
#  EGL_FOUND - Set to true if egl has been found
#  EGL_INCLUDE_DIRS - The egl include directories
#  EGL_LIBRARIES - The libraries needed to use egl

find_package(PkgConfig)
pkg_check_modules(PC_EGL QUIET egl)

find_path(EGL_INCLUDE_DIR
        NAMES EGL/egl.h
        HINTS ${PC_EGL_INCLUDEDIR} ${PC_EGL_INCLUDE_DIRS})
find_library(EGL_LIBRARY
        NAMES EGL libEGL
        HINTS ${PC_EGL_LIBDIR} ${PC_EGL_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EGL DEFAULT_MSG
        EGL_LIBRARY EGL_INCLUDE_DIR)

mark_as_advanced(EGL_INCLUDE_DIR EGL_LIBRARY)

set(EGL_LIBRARIES ${EGL_LIBRARY})
set(EGL_INCLUDE_DIRS ${EGL_INCLUDE_DIR})
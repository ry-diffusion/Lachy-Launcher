# Finds the `evdev` library.
# This file is released under the Public Domain.
# Once done this will define
#  EVDEV_FOUND - Set to true if evdev has been found
#  EVDEV_INCLUDE_DIRS - The evdev include directories
#  EVDEV_LIBRARIES - The libraries needed to use evdev

find_package(PkgConfig)
pkg_check_modules(PC_EVDEV QUIET libevdev)

find_path(EVDEV_INCLUDE_DIR
        NAMES libevdev/libevdev.h
        PATH_SUFFIXES libevdev-1.0
        HINTS ${PC_EVDEV_INCLUDEDIR} ${PC_EVDEV_INCLUDE_DIRS})
find_library(EVDEV_LIBRARY
        NAMES evdev libevdev
        HINTS ${PC_EVDEV_LIBDIR} ${PC_EVDEV_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EVDEV DEFAULT_MSG
        EVDEV_LIBRARY EVDEV_INCLUDE_DIR)

mark_as_advanced(EVDEV_INCLUDE_DIR EVDEV_LIBRARY)

set(EVDEV_LIBRARIES ${EVDEV_LIBRARY})
set(EVDEV_INCLUDE_DIRS ${EVDEV_INCLUDE_DIR})
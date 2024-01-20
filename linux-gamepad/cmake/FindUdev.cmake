# Finds the `udev` library.
# This file is released under the Public Domain.
# Once done this will define
#  UDEV_FOUND - Set to true if evdev has been found
#  UDEV_INCLUDE_DIRS - The evdev include directories
#  UDEV_LIBRARIES - The libraries needed to use evdev

find_package(PkgConfig)
pkg_check_modules(PC_UDEV QUIET libevdev)

find_path(UDEV_INCLUDE_DIR
        NAMES libudev.h
        PATH_SUFFIXES libudev
        HINTS ${PC_UDEV_INCLUDEDIR} ${PC_UDEV_INCLUDE_DIRS})
find_library(UDEV_LIBRARY
        NAMES udev libudev
        HINTS ${PC_UDEV_LIBDIR} ${PC_UDEV_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UDEV DEFAULT_MSG
        UDEV_LIBRARY UDEV_INCLUDE_DIR)

mark_as_advanced(UDEV_INCLUDE_DIR UDEV_LIBRARY)

set(UDEV_LIBRARIES ${UDEV_LIBRARY})
set(UDEV_INCLUDE_DIRS ${UDEV_INCLUDE_DIR})
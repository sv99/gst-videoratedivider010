cmake_minimum_required(VERSION 3.1)
project(libgstvideoratedivider)
set(VERSION 0.1.0)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

configure_file(${SRC_DIR}/config.h.in ${SRC_DIR}/config.h)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GSTRM REQUIRED gstreamer-0.10)
pkg_check_modules(GSTRMAPP REQUIRED gstreamer-app-0.10)

include_directories(
        ${GLIB2_INCLUDE_DIRS}
        ${GSTRM_INCLUDE_DIRS}
        ${GSTRMAPP_INCLUDE_DIRS})
link_directories(
        ${GLIB2_LIBRARY_DIRS}
        ${GSTRM_LIBRARY_DIRS}
        ${GSTRMAPP_LIBRARY_DIRS})

# message(STATUS ${GSTRM_INCLUDE_DIRS})

add_subdirectory(src)
# add_subdirectory(tests)

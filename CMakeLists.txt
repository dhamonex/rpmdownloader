cmake_minimum_required (VERSION 2.8)
project(rpmdownloader)

set(rpmdownloader_VERSION 0.11.3)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

find_package( Qt4 REQUIRED )
find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})

set( QT_USE_QTXML TRUE )
set( QT_USE_QTSQL TRUE )

configure_file(version.h.cmake version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(src)
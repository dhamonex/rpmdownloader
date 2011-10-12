cmake_minimum_required (VERSION 2.6)
project(rpmdownloader)

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

find_package( Qt4 REQUIRED )

set( QT_USE_QTXML TRUE )
set( QT_USE_QTNETWORK TRUE )
set( QT_USE_QTSQL TRUE )

add_subdirectory(src)
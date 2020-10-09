cmake_minimum_required(VERSION 3.5)

project(rlpc LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(WIN32)
set(CMAKE_CXX_FLAGS "-DTAGLIB_STATIC")
endif()

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
                include_directories("/usr/local/include/" ${CMAKE_SOURCE_DIR})
                link_directories("/usr/local/lib/")
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Multimedia REQUIRED)
find_package(Qt5 COMPONENTS Svg REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

#if(ANDROID)
#  add_library(rlpc SHARED
#    main.cpp
#    rlpcmain.cpp
#    rlpcmain.hpp
#    rlpcmain.ui
#    search.ui
#    yandex_api.h
#    yandex_api.c
#    search.cpp
#    search.hpp
#  )
#else()
  add_executable(rlpc
    main.cpp
    rlpcmain.cpp
    rlpcmain.hpp
    rlpcmain.ui
    yandexmusic.h
  )
#endif()
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
target_link_libraries(rlpc PRIVATE Qt5::Widgets Qt5::Multimedia Qt5::Svg tag ${CMAKE_SOURCE_DIR/libyandexmusic.so})
endif()
if(WIN32)
target_link_libraries(rlpc PRIVATE Qt5::Widgets Qt5::Multimedia Qt5::Svg ${CMAKE_SOURCE_DIR}/libtag.a ${CMAKE_SOURCE_DIR}/libyandexmusic.a)
endif()
if(UNIX)
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
target_link_libraries(rlpc PRIVATE Qt5::Widgets Qt5::Multimedia Qt5::Svg tag ${CMAKE_SOURCE_DIR}/libyandexmusic.so)
elseif()
target_link_libraries(rlpc PRIVATE Qt5::Widgets Qt5::Multimedia Qt5::Svg tag yandexmusic)
endif()
endif()

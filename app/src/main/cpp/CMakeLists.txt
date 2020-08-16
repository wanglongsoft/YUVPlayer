
cmake_minimum_required(VERSION 3.4.1)

file(GLOB allCpp ${CMAKE_SOURCE_DIR}/*.cpp)

include_directories(${CMAKE_SOURCE_DIR}/ffmpeg/include)
include_directories(${CMAKE_SOURCE_DIR}/rtmp/include)
include_directories(${CMAKE_SOURCE_DIR}/libyuv/include)

LINK_DIRECTORIES("${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}/")

add_library(
        native-lib
        SHARED
        ${allCpp} )

find_library(
        log-lib
        log )


target_link_libraries(
        native-lib
        -Wl,--start-group
        avcodec avfilter avformat avutil swresample swscale postproc avdevice
        -Wl,--end-group
        rtmp
        EGL
        GLESv3
        android
        yuv
        ${log-lib} )
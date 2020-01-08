
cmake_minimum_required(VERSION 3.8)

set(TARGET_NAME FFMpeg_Common)

add_library(${TARGET_NAME} SHARED ffmpeg_common.h ffmpeg_common.cpp)

target_link_libraries(${TARGET_NAME}
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        postproc
        swresample
        swscale
        SDL2
        SDL2main
        )

add_dependencies(${TARGET_NAME} copy_runtime)


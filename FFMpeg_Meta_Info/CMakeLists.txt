
cmake_minimum_required(VERSION 3.8)

set(TARGET_NAME FFMpeg_Meta_Info)

add_executable(${TARGET_NAME} ffmpeg_meta_info.c)

target_link_libraries(${TARGET_NAME}
	avcodec
	avdevice
	avfilter
	avformat
	avutil
	postproc
	swresample
	swscale
)


add_dependencies(${TARGET_NAME} copy_runtime)


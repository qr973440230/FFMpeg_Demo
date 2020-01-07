
cmake_minimum_required(VERSION 3.8)

set(TARGET_NAME FFMpeg_Get_Video)

add_executable(${TARGET_NAME} ffmpeg_get_video.c)

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


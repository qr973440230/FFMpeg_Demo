
cmake_minimum_required(VERSION 3.8)

set(TARGET_NAME FFMpeg_Get_Audio)

add_executable(${TARGET_NAME} ffmpeg_get_audio.c)

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



cmake_minimum_required(VERSION 3.8)

add_executable(FFMpeg_Configuration ffmpeg_configuration.cpp)

target_link_libraries(FFMpeg_Configuration
	avcodec
	avdevice
	avfilter
	avformat
	avutil
	postproc
	swresample
	swscale
)
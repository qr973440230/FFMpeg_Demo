# CMakeList.txt: FFMpeg_Demo 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

set(TARGET_NAME FFMpeg_Version)

# 将源代码添加到此项目的可执行文件。
add_executable (${TARGET_NAME} ffmpeg_version.c)

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


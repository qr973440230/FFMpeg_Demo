# CMakeList.txt: FFMpeg_Demo 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 将源代码添加到此项目的可执行文件。
add_executable (FFMpeg_Version "ffmpeg_version.c")

target_link_libraries(FFMpeg_Version 
	avcodec
	avdevice
	avfilter
	avformat
	avutil
	postproc
	swresample
	swscale
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
add_custom_command(TARGET FFMpeg_Version POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${PROJECT_SOURCE_DIR}/Shared/x64"
	$<TARGET_FILE_DIR:FFMpeg_Version>
)
else()
add_custom_command(TARGET FFMpeg_Version POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${PROJECT_SOURCE_DIR}/Shared/x86"
	$<TARGET_FILE_DIR:FFMpeg_Version>
)
endif()
# TODO: 如有需要，请添加测试并安装目标。


cmake_minimum_required(VERSION 3.8)

add_executable(FFMpeg_Configuration ffmpeg_configuration.c)

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

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
add_custom_command(TARGET FFMpeg_Configuration POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${PROJECT_SOURCE_DIR}/Shared/x64"
	$<TARGET_FILE_DIR:FFMpeg_Configuration>
)
else()
add_custom_command(TARGET FFMpeg_Configuration POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${PROJECT_SOURCE_DIR}/Shared/x86"
	$<TARGET_FILE_DIR:FFMpeg_Configuration>
)
endif()

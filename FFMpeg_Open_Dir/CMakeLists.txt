
cmake_minimum_required(VERSION 3.8)

set(TARGET_NAME FFMpeg_Open_Dir)

add_executable(${TARGET_NAME} ffmpeg_open_dir.c)

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

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${PROJECT_SOURCE_DIR}/Shared/x64"
	$<TARGET_FILE_DIR:${TARGET_NAME}>
)
else()
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${PROJECT_SOURCE_DIR}/Shared/x86"
	$<TARGET_FILE_DIR:${TARGET_NAME}>
)
endif()

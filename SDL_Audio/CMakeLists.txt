# CMakeList.txt: FFMpeg_Demo 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

set(TARGET_NAME SDL_Audio)

# 将源代码添加到此项目的可执行文件。
add_executable (${TARGET_NAME} sdl_audio.c)

target_link_libraries(${TARGET_NAME}
	SDL2
	SDL2main
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
# TODO: 如有需要，请添加测试并安装目标。

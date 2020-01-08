# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("FFMpeg_Demo")

#设置exe 输出目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
#设置lib dll输出目录
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

#设置include
include_directories(include/ffmpeg include/sdl2 FFMpeg_Common)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    link_directories(lib/x64)
    add_custom_target(copy_runtime ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/shared/x64" "${CMAKE_BINARY_DIR}"
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resource" "${CMAKE_BINARY_DIR}"
	)
else()
    link_directories(lib/x86)
    add_custom_target(copy_runtime ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/shared/x86" "${CMAKE_BINARY_DIR}"
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resource" "${CMAKE_BINARY_DIR}"
	)
endif()

# 包含子项目。
add_subdirectory (FFMpeg_Version)

add_subdirectory(FFMpeg_Configuration)

add_subdirectory(FFMpeg_Open_Dir)

add_subdirectory(FFMpeg_Meta_Info)

add_subdirectory(FFMpeg_Common)

add_subdirectory(FFMpeg_Mp4_Flv)

add_subdirectory(FFMpeg_Mp4_Cut)

add_subdirectory(FFMpeg_Decoding)

add_subdirectory(FFMpeg_Encoding)

add_subdirectory(FFMpeg_SDL_Video)

add_subdirectory(FFMpeg_SDL_Audio)

add_subdirectory(FFMpeg_SDL_Player)

# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("FFMpeg_Demo")

include_directories(
	"Third_Party_Libraries/FFMpeg/include"
	"Third_Party_Libraries/SDL/include"
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
link_directories(
	"Third_Party_Libraries/FFMpeg/lib/x64"
	"Third_Party_Libraries/SDL/lib/x64"
)
else()
link_directories(
	"Third_Party_Libraries/FFMpeg/lib/x86"
	"Third_Party_Libraries/SDL/lib/x86"
)
endif()


# 包含子项目。
add_subdirectory ("FFMpeg_Version")

add_subdirectory("FFMpeg_Configuration")

add_subdirectory("SDL_Video")

add_subdirectory("SDL_Audio")

add_subdirectory("FFMpeg_SDL_Video")

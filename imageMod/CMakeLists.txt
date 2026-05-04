cmake_minimum_required(VERSION 4.0)
project(ImageMod LANGUAGES C)

add_library(ImageMod SHARED)

target_compile_features(ImageMod PRIVATE c_std_17)

target_sources(ImageMod
	PRIVATE
		src/imageMod.c

	PUBLIC
		FILE_SET HEADERS
		FILES
			src/main.h
)

set_target_properties(ImageMod PROPERTIES PREFIX "")

if(TARGET_WIN)
	target_compile_definitions(ImageMod PRIVATE BUILD_DLL)
endif()

install(
	TARGETS ImageMod
	FILE_SET HEADERS
)
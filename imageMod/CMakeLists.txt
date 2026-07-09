cmake_minimum_required(VERSION 4.0)
project(ImageMod LANGUAGES C)

add_library(ImageMod SHARED)

target_compile_features(ImageMod PRIVATE c_std_17)

target_sources(ImageMod
	PRIVATE
		src/imageMod.c
		src/imageMod.def

	PUBLIC
		FILE_SET HEADERS
		FILES
			src/imageMod.h
)

set_target_properties(ImageMod PROPERTIES PREFIX "")

install(
	TARGETS ImageMod
	FILE_SET HEADERS
)
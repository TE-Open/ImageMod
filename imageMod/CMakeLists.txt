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
			src/imageMod.h
)

if(WIN32)
	target_sources(ImageMod PRIVATE src/imageMod.def)
elseif(UNIX)
	target_link_options(ImageMod PRIVATE "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/imageMod.map")
	set_target_properties(ImageMod PROPERTIES LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/imageMod.map")
endif()

set_target_properties(ImageMod PROPERTIES PREFIX "")

install(
	TARGETS ImageMod
	FILE_SET HEADERS
)
cmake_minimum_required(VERSION 2.8)
project(utuclient2 C)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

set(CMAKE_C_FLAGS "-Wall -std=c99")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -Werror -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O2 -fno-omit-frame-pointer -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")

find_package(argtable2)
find_package(SDL2)
find_package(ffmpeg COMPONENTS avcodec avformat avutil swscale swresample)
find_package(websockets)
find_package(openal)

include_directories(
	include
	${ARGTABLE2_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIRS}
	${FFMPEG_INCLUDE_DIRS}
	${OPENAL_INCLUDE_DIR}
)

add_executable(utuclient2
	src/args.c
	src/video.c
	src/audio.c
	src/player.c
	src/framework.c
	src/engine.c
	src/ringbuffer.c
	src/main.c
)

if(MINGW)
    target_link_libraries(utuclient2 mingw32)
endif()

target_link_libraries(utuclient2
	${ARGTABLE2_LIBRARIES}
	${SDL2_LIBRARIES}
	${FFMPEG_LIBRARIES}
	${OPENAL_LIBRARY}
)

# Installation
INSTALL(TARGETS utuclient2
    RUNTIME
    DESTINATION bin
)

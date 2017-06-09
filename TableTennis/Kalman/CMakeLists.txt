cmake_minimum_required(VERSION 2.8)
project(TableTennis)
find_package(OpenCV REQUIRED)

find_path(KALMAN_INCLUDE_DIR
	Kalman.h
	HINTS
	./include)
	
include_directories(${KALMAN_INCLUDE_DIR})

# Find background subtraction library
find_path(Codebook_INCLUDE_DIR
	Codebook.h
        HINTS
        ../BgSubtractor/include)

find_library(Codebook_LIBRARY
	bgsubtract
        HINTS
        ../build/BgSubtractor)

include_directories(${Codebook_INCLUDE_DIR})

add_executable(kalman main.cpp Kalman.cpp)
target_link_libraries(kalman ${OpenCV_LIBS})

add_executable(track track.cpp)
target_link_libraries(track ${OpenCV_LIBS} ${Codebook_LIBRARY})

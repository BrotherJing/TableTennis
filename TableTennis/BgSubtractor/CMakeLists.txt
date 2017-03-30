cmake_minimum_required(VERSION 2.8)
project(TableTennis)
find_package(OpenCV REQUIRED)

find_path(BG_SUBTRACT_INCLUDE_DIR
	Codebook.h
	HINTS
	./include)
	
include_directories(${BG_SUBTRACT_INCLUDE_DIR})

add_executable(bg Main.cpp Codebook.cpp ConnectedComp.cpp Track.cpp)
target_link_libraries(bg ${OpenCV_LIBS})

add_library(bgsubtract SHARED Codebook.cpp ConnectedComp.cpp Track.cpp)
target_link_libraries(bgsubtract ${OpenCV_LIBS})

add_executable(CropImage CropImage.cpp CropUtils.cpp Codebook.cpp ConnectedComp.cpp Track.cpp)
target_link_libraries(CropImage ${OpenCV_LIBS})

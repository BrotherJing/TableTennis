cmake_minimum_required(VERSION 2.8)
project(TableTennis)
find_package(OpenCV REQUIRED)
add_executable(TableTennis Main.cpp Table.cpp Hist.cpp Draw.cpp Reconstruct3D.cpp)
target_link_libraries(TableTennis ${OpenCV_LIBS})

add_executable(bg ./BgSubtractor/Main.cpp ./BgSubtractor/Codebook.cpp ./BgSubtractor/ConnectedComp.cpp ./BgSubtractor/Track.cpp)
target_link_libraries(bg ${OpenCV_LIBS})

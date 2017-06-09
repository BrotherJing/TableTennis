cmake_minimum_required(VERSION 2.8)
project(TableTennis)
find_package(OpenCV REQUIRED)

add_subdirectory(BgSubtractor)
add_subdirectory(Reconstruct3D)
add_subdirectory(Kalman)

add_executable(TableTennis Main.cpp Table.cpp Hist.cpp Draw.cpp CalcCameraMatrix.cpp)
target_link_libraries(TableTennis ${OpenCV_LIBS})

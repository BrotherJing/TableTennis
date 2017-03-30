cmake_minimum_required(VERSION 2.8)
project(TableTennis)
find_package(OpenCV REQUIRED)

add_executable(reconstruct Main.cpp)
add_executable(remaptest RemapTest.cpp)
target_link_libraries(reconstruct ${OpenCV_LIBS})
target_link_libraries(remaptest ${OpenCV_LIBS})

add_executable(GetZScale GetZScale.cpp)
target_link_libraries(GetZScale ${OpenCV_LIBS})

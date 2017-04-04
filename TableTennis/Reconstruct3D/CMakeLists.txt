cmake_minimum_required(VERSION 2.8)
project(TableTennis)
find_package(OpenCV REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(rec3d SHARED Reconstruct.cpp)
target_link_libraries(rec3d ${OpenCV_LIBS})

add_executable(reconstruct Main.cpp Reconstruct.cpp)
target_link_libraries(reconstruct ${OpenCV_LIBS})

add_executable(remaptest RemapTest.cpp)
target_link_libraries(remaptest ${OpenCV_LIBS})

add_executable(GetZScale GetZScale.cpp)
target_link_libraries(GetZScale ${OpenCV_LIBS})

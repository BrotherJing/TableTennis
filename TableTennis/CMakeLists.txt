cmake_minimum_required(VERSION 2.8)
project(TableTennis)
find_package(OpenCV REQUIRED)
add_executable(TableTennis Main.cpp Table.cpp Hist.cpp Draw.cpp CalcCameraMatrix.cpp)
target_link_libraries(TableTennis ${OpenCV_LIBS})

add_executable(bg ./BgSubtractor/Main.cpp ./BgSubtractor/Codebook.cpp ./BgSubtractor/ConnectedComp.cpp ./BgSubtractor/Track.cpp)
target_link_libraries(bg ${OpenCV_LIBS})

add_library(bgsubtract SHARED ./BgSubtractor/Codebook.cpp ./BgSubtractor/ConnectedComp.cpp ./BgSubtractor/Track.cpp)
target_link_libraries(bgsubtract ${OpenCV_LIBS})

add_executable(reconstruct ./Reconstruct3D/Main.cpp)
add_executable(remaptest ./Reconstruct3D/RemapTest.cpp)
target_link_libraries(reconstruct ${OpenCV_LIBS})
target_link_libraries(remaptest ${OpenCV_LIBS})

add_executable(GetZScale ./Reconstruct3D/GetZScale.cpp)
target_link_libraries(GetZScale ${OpenCV_LIBS})

add_executable(CropImage ./BgSubtractor/CropImage.cpp ./BgSubtractor/CropUtils.cpp ./BgSubtractor/Codebook.cpp ./BgSubtractor/ConnectedComp.cpp ./BgSubtractor/Track.cpp)
target_link_libraries(CropImage ${OpenCV_LIBS})
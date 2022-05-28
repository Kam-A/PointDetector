cmake_minimum_required(VERSION 3.1)
project(PointDetector)

find_package(xtl REQUIRED)
find_package(xtensor REQUIRED)
find_package(xtensor-blas REQUIRED)

add_executable(point_detector src/main.cpp src/line.h src/detector.h src/detector.cpp src/basket.cpp src/basket.h src/input_reader.cpp src/input_reader.h)

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
endif()

target_link_libraries(point_detector xtensor xtensor-blas xtensor::optimize)

cmake_minimum_required(VERSION 3.24)

project(ImageToGCode)

set(CMAKE_CXX_STANDARD 23)

add_compile_definitions(UNICODE)

include_directories(Common)

list(APPEND CMAKE_PREFIX_PATH "~/opencv/build/x64/vc16/lib")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS}) # Not needed for CMake >= 2.8.11
link_libraries(${OpenCV_LIBS})

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 灰度图像转GCode
add_executable(ImageToGCode main.cpp Common.hpp ImageToGCode.h ImageToGCode.cpp)

# 基本G0和G1指令
add_executable(G0G1Impl g0g1impl.cpp)

# 单向
add_executable(UnidirectionalScanning UnidirectionalScanning/main.cpp
                                      Common/Plane.h)

# 双向
add_executable(BidirectionalScanning BidirectionalScanning/main.cpp
                                     Common/Plane.h)

# 斜向
add_executable(DiagonalScanning DiagonalScanning/main.cpp Common/Plane.h)

# 螺旋
add_executable(SpiralScanning SpiralScanning/main.cpp Common/Plane.h)

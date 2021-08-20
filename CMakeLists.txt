cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

project(V4L2Viewer VERSION 1.0)

include(V4L2ViewerLib.cmake)

add_executable(V4L2Viewer Source/main.cpp)
target_link_libraries(V4L2Viewer V4L2ViewerLib)

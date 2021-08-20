cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

set(MAIN_EXEC Source/main.cpp)

project(V4L2Viewer VERSION 1.0)
set(TARGET_NAME V4L2Viewer)

add_subdirectory(V4L2StaticLibrary)
set(LIBRARIES V4L2StaticLib)

add_executable(${TARGET_NAME} ${MAIN_EXEC})
target_link_libraries(${TARGET_NAME} ${LIBRARIES})

cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-std=c++17)

set(MAIN_EXEC Source/main.cpp)

project(V4L2_Viewer VERSION 1.0)
set(TARGET_NAME V4L2_Viewer)

add_subdirectory(V4L2StaticLibrary)
set(LIBRARIES V4L2StaticLib)

include_directories(V4L2StaticLibrary)
link_directories(V4L2StaticLibrary)

find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
find_package(Threads REQUIRED)

list(APPEND QT_LIBRARIES
Qt5::Core
Qt5::Gui
Qt5::Widgets
Threads::Threads
)

add_executable(${TARGET_NAME} ${MAIN_EXEC})

target_include_directories(${TARGET_NAME} PUBLIC Main/include ${CMAKE_CURRENT_BINARY_DIR}/V4L2StaticLibrary/V4L2StaticLib_autogen/include)
target_link_libraries(${TARGET_NAME} ${LIBRARIES} ${QT_LIBRARIES})

target_include_directories(${TARGET_NAME} PUBLIC V4L2StaticLibrary)


execute_process(COMMAND 
	git rev-parse HEAD
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	OUTPUT_VARIABLE GIT_SHA1
)
string(REGEX REPLACE "\n$" "" GIT_SHA1 "${GIT_SHA1}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Source/Headers/Version.h.template" "${CMAKE_CURRENT_SOURCE_DIR}/Source/Headers/Version.h" @ONLY)

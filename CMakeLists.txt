#-------------------------------------------------------------------------------
#	CMAKE SETTINGS
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.8)
set(QT_MINIMUM_VERSION 5.0)
set(HEADERS_PATH Source/Headers)
set(SOURCES_PATH Source/Source)
set(RESOURCES_PATH Source/Resources)

#-------------------------------------------------------------------------------
#	SOURCE CODE
#-------------------------------------------------------------------------------

# .h files
list(APPEND HEADER_FILES
${HEADERS_PATH}/BaseLogger.h
${HEADERS_PATH}/Camera.h
${HEADERS_PATH}/CameraObserver.h
${HEADERS_PATH}/DeviationCalculator.h
${HEADERS_PATH}/FrameObserver.h
${HEADERS_PATH}/FrameObserverMMAP.h
${HEADERS_PATH}/FrameObserverRead.h
${HEADERS_PATH}/FrameObserverUSER.h
${HEADERS_PATH}/ImageProcessingThread.h
${HEADERS_PATH}/ImageTransform.h
${HEADERS_PATH}/IOHelper.h
${HEADERS_PATH}/LocalMutex.h
${HEADERS_PATH}/LocalMutexLockGuard.h
${HEADERS_PATH}/Logger.h
${HEADERS_PATH}/MemoryHelper.h
${HEADERS_PATH}/MyFrame.h
${HEADERS_PATH}/MyFrameQueue.h
${HEADERS_PATH}/Thread.h
${HEADERS_PATH}/V4L2Helper.h
${HEADERS_PATH}/V4L2Viewer.h
${HEADERS_PATH}/videodev2_av.h
)

# .cpp files
list(APPEND SOURCE_FILES
${SOURCES_PATH}/BaseLogger.cpp
${SOURCES_PATH}/Camera.cpp
${SOURCES_PATH}/CameraObserver.cpp
${SOURCES_PATH}/DeviationCalculator.cpp
${SOURCES_PATH}/FrameObserver.cpp
${SOURCES_PATH}/FrameObserverMMAP.cpp
${SOURCES_PATH}/FrameObserverRead.cpp
${SOURCES_PATH}/FrameObserverUSER.cpp
${SOURCES_PATH}/ImageProcessingThread.cpp
${SOURCES_PATH}/ImageTransform.cpp
${SOURCES_PATH}/IOHelper.cpp
${SOURCES_PATH}/Logger.cpp
${SOURCES_PATH}/main.cpp
${SOURCES_PATH}/MyFrame.cpp
${SOURCES_PATH}/MyFrameQueue.cpp
${SOURCES_PATH}/Thread.cpp
${SOURCES_PATH}/V4L2Helper.cpp
${SOURCES_PATH}/V4L2Viewer.cpp
)

set(ALL_SOURCE ${SOURCE_FILES} ${HEADER_FILES})

#-------------------------------------------------------------------------------
#	PROJECT SETTINGS
#-------------------------------------------------------------------------------
project(V4L2_Viewer VERSION 1.0)
set(TARGET_NAME V4L2_Viewer)

math(EXPR PLATFORM_BITS "${CMAKE_SIZEOF_VOID_P} * 8")
set(PLATFORM_OUT_DIR ${CMAKE_SYSTEM_NAME}${PLATFORM_BITS}-${CMAKE_CXX_COMPILED_ID}${CMAKE_CXX_COMPILER_VERSION}-${CMAKE_BUILD_TYPE})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PLATFORM_OUT_DIR})

#-------------------------------------------------------------------------------
#	Qt Settings
#-------------------------------------------------------------------------------

# Required Qt settings

set(CMAKE_AUTOUIC_SEARCH_PATHS ${RESOURCES_PATH}/Forms)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

# find thread library
find_package(Threads REQUIRED)

#-------------------------------------------------------------------------------
#	RESOURCES
#-------------------------------------------------------------------------------

list(APPEND QT_RESOURCES
${RESOURCES_PATH}/V4L2Viewer.qrc
)

qt5_add_resources(RESOURCES ${QT_RESOURCES})

list(APPEND RESOURCES
${RESOURCES_PATH}/V4L2Viewer.rc
)

#-------------------------------------------------------------------------------
#	COMPILE FLAGS
#-------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-std=c++17)

#-------------------------------------------------------------------------------
#	SOURCE GROUPS
#-------------------------------------------------------------------------------

# source_group(generated REGULAR_EXPRESSION ".*_automoc.cpp")
# source_group(resources REGULAR_EXPRESSION ".*[.][q]?rc")
# source_group(source REGULAR_EXPRESSION ".*[.]cpp")
# source_group(headers REGULAR_EXPRESSION ".*[.]h")
# source_group(continuous-integration REGULAR_EXPRESSION ".*[.]yml")

#-------------------------------------------------------------------------------
#	ADDITIONAL LIBRARIES
#-------------------------------------------------------------------------------

list(APPEND LIBRARIES
Qt5::Core
Qt5::Gui
Qt5::Widgets
Threads::Threads
)

# Remove duplicate library dependencies. 
list(REMOVE_DUPLICATES LIBRARIES)

#-------------------------------------------------------------------------------
#	GENERATE EXECUTABLE
#-------------------------------------------------------------------------------

# Generate the executable
add_executable(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCES})

# Set additional include directories

target_include_directories(${TARGET_NAME} PRIVATE ${HEADERS_PATH})

# link libraries
target_link_libraries(${TARGET_NAME} ${LIBRARIES})

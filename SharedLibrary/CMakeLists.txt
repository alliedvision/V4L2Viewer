set(HEADERS_PATH ../Source/Headers)
set(SOURCES_PATH ../Source/Source)
set(RESOURCES_PATH ../Source/Resources)

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
${SOURCES_PATH}/MyFrame.cpp
${SOURCES_PATH}/MyFrameQueue.cpp
${SOURCES_PATH}/Thread.cpp
${SOURCES_PATH}/V4L2Helper.cpp
${SOURCES_PATH}/V4L2Viewer.cpp
)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${RESOURCES_PATH}/Forms)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
find_package(Threads REQUIRED)

list(APPEND QT_LIBRARIES
Qt5::Core
Qt5::Gui
Qt5::Widgets
Threads::Threads
)

list(APPEND QT_RESOURCES
${RESOURCES_PATH}/V4L2Viewer.qrc
)

qt5_add_resources(RESOURCES ${QT_RESOURCES})

list(APPEND RESOURCES
${RESOURCES_PATH}/V4L2Viewer.rc
)

add_library(SharedLibraryLib SHARED ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCES})
target_link_libraries(SharedLibraryLib ${QT_LIBRARIES})
target_include_directories(SharedLibraryLib PUBLIC ${HEADERS_PATH})






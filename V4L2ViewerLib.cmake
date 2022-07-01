# Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
# Copyright (C) 2021 Allied Vision Technologies GmbH
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

set(HEADERS_PATH Source/Headers)
set(SOURCES_PATH Source/Source)
set(RESOURCES_PATH Source/Resources)
set(ENUM_INT_PATH ${HEADERS_PATH}/EnumeratorInterface)
set(GIT_REVISION_FILE ${CMAKE_CURRENT_BINARY_DIR}/GitRevision.h)

list(APPEND HEADER_FILES
  ${HEADERS_PATH}/BaseLogger.h
  ${HEADERS_PATH}/Camera.h
  ${HEADERS_PATH}/CameraObserver.h
  ${HEADERS_PATH}/FrameObserver.h
  ${HEADERS_PATH}/FrameObserverMMAP.h
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
  ${HEADERS_PATH}/SelectSubDeviceDialog.h
  ${HEADERS_PATH}/Thread.h
  ${HEADERS_PATH}/V4L2Helper.h
  ${HEADERS_PATH}/V4L2Viewer.h
  ${HEADERS_PATH}/videodev2_av.h
  ${HEADERS_PATH}/AboutWidget.h
  ${HEADERS_PATH}/CameraListCustomItem.h
  ${HEADERS_PATH}/AutoReader.h
  ${HEADERS_PATH}/AutoReaderWorker.h
  ${HEADERS_PATH}/EnumeratorInterface/IControlEnumerationHolder.h
  ${HEADERS_PATH}/EnumeratorInterface/IntegerEnumerationControl.h
  ${HEADERS_PATH}/EnumeratorInterface/Integer64EnumerationControl.h
  ${HEADERS_PATH}/EnumeratorInterface/ControlsHolderWidget.h
  ${HEADERS_PATH}/EnumeratorInterface/BooleanEnumerationControl.h
  ${HEADERS_PATH}/EnumeratorInterface/ButtonEnumerationControl.h
  ${HEADERS_PATH}/EnumeratorInterface/ListEnumerationControl.h
  ${HEADERS_PATH}/EnumeratorInterface/ListIntEnumerationControl.h
  ${HEADERS_PATH}/CustomGraphicsView.h
  ${HEADERS_PATH}/CustomDialog.h
  ${HEADERS_PATH}/V4L2EventHandler.h
  ${HEADERS_PATH}/FPSCalculator.h
)

list(APPEND SOURCE_FILES
  ${SOURCES_PATH}/BaseLogger.cpp
  ${SOURCES_PATH}/Camera.cpp
  ${SOURCES_PATH}/CameraObserver.cpp
  ${SOURCES_PATH}/FrameObserver.cpp
  ${SOURCES_PATH}/FrameObserverMMAP.cpp
  ${SOURCES_PATH}/FrameObserverUSER.cpp
  ${SOURCES_PATH}/ImageProcessingThread.cpp
  ${SOURCES_PATH}/ImageTransform.cpp
  ${SOURCES_PATH}/IOHelper.cpp
  ${SOURCES_PATH}/Logger.cpp
  ${SOURCES_PATH}/MyFrame.cpp
  ${SOURCES_PATH}/MyFrameQueue.cpp
  ${SOURCES_PATH}/SelectSubDeviceDialog.cpp
  ${SOURCES_PATH}/Thread.cpp
  ${SOURCES_PATH}/V4L2Helper.cpp
  ${SOURCES_PATH}/V4L2Viewer.cpp
  ${SOURCES_PATH}/AboutWidget.cpp
  ${SOURCES_PATH}/CameraListCustomItem.cpp
  ${SOURCES_PATH}/AutoReader.cpp
  ${SOURCES_PATH}/AutoReaderWorker.cpp
  ${SOURCES_PATH}/EnumeratorInterface/IControlEnumerationHolder.cpp
  ${SOURCES_PATH}/EnumeratorInterface/IntegerEnumerationControl.cpp
  ${SOURCES_PATH}/EnumeratorInterface/Integer64EnumerationControl.cpp
  ${SOURCES_PATH}/EnumeratorInterface/ControlsHolderWidget.cpp
  ${SOURCES_PATH}/EnumeratorInterface/BooleanEnumerationControl.cpp
  ${SOURCES_PATH}/EnumeratorInterface/ButtonEnumerationControl.cpp
  ${SOURCES_PATH}/EnumeratorInterface/ListEnumerationControl.cpp
  ${SOURCES_PATH}/EnumeratorInterface/ListIntEnumerationControl.cpp
  ${SOURCES_PATH}/CustomGraphicsView.cpp
  ${SOURCES_PATH}/CustomDialog.cpp
  ${SOURCES_PATH}/V4L2EventHandler.cpp
  ${SOURCES_PATH}/FPSCalculator.cpp
  ${GIT_REVISION_FILE}
)

set_property(SOURCE ${GIT_REVISION_FILE} PROPERTY SKIP_AUTOGEN ON)

add_custom_command(
  OUTPUT ${GIT_REVISION_FILE}
  COMMAND ${CMAKE_COMMAND} -E remove -f ${GIT_REVISION_FILE}
  COMMAND ${CMAKE_COMMAND} -E echo_append "#define GIT_VERSION " >> ${GIT_REVISION_FILE}
  COMMAND git log -1 "--pretty=format:\"%H\"" >> ${GIT_REVISION_FILE}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DEPENDS ${CMAKE_SOURCE_DIR}/.git/logs/HEAD
  VERBATIM
)
list(APPEND HEADER_FILES ${GIT_REVISION_FILE})


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
  ${RESOURCES_PATH}/Forms/ControlsHolderWidget.ui
  ${RESOURCES_PATH}/Forms/V4L2Viewer.ui
  ${RESOURCES_PATH}/Forms/ActiveExposureWidget.ui
  ${RESOURCES_PATH}/Forms/AboutWidget.ui
  ${RESOURCES_PATH}/Forms/CustomDialog.ui
)

add_library(V4L2ViewerLib STATIC ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCES})

target_link_libraries(V4L2ViewerLib PUBLIC ${QT_LIBRARIES})
target_include_directories(V4L2ViewerLib
  PUBLIC
    ${HEADERS_PATH}
    ${ENUM_INT_PATH}
    ${CMAKE_CURRENT_BINARY_DIR}/V4L2ViewerLib_autogen/include
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

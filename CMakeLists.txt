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

cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0095 NEW)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

project(V4L2Viewer VERSION 2.2.1)

add_subdirectory(lib)


add_executable(V4L2Viewer Source/main.cpp)
target_link_libraries(V4L2Viewer V4L2ViewerLib)
set_target_properties(V4L2Viewer PROPERTIES INSTALL_RPATH "$ORIGIN")




install(TARGETS V4L2Viewer DESTINATION .)
install(FILES LICENSE.md README.rst DESTINATION .)
install(DIRECTORY $ENV{QT_5_15_arm64_DEPLOY}/ DESTINATION . FILES_MATCHING PATTERN *)
install(FILES $ENV{QT_5_15_arm64}/lib/libQt5Widgets.so.5 DESTINATION .)
install(FILES $ENV{QT_5_15_arm64}/lib/libQt5Widgets.so.5.15.14 DESTINATION .)

set(CPACK_GENERATOR TGZ)

include(CPack)
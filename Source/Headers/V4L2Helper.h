/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2021 Allied Vision Technologies GmbH

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.  */


#ifndef V4L2HELPER_H
#define V4L2HELPER_H

#include <stdint.h>

#include <string>

namespace v4l2helper
{

// This function converts errors to string and returns it
//
// Parameters:
// [in] (int) errnumber - error number
//
// Returns:
// std::string - error
std::string ConvertErrno2String(int errnumber);
// This function converts pixel format to string and returns it
//
// Parameters:
// [in] (int) pixelFormat - pixel format
//
// Returns:
// std::string - pixelFormat
std::string ConvertPixelFormat2String(int pixelFormat);
// Function converts pixel format from integer to string
//
// Parameters:
// [in] (int) pixelFormat - given pixel format
//
// Returns:
// (std::string) - string variable representing pixel format
std::string ConvertPixelFormat2EnumString(int pixelFormat);
// This function converts available controls' id to string
//
// Parameters:
// [in] (uint32_t) controlID - given control id
//
// Returns:
// (std::string) - string variable representing control's id
std::string ConvertControlID2String(uint32_t controlID);
// This function returns control unit, it is used in the
// function which enumerates all control
//
// Parameters:
// [in] (int32_t) id - given id
//
// Returns:
// (std::string) - unit of the control
std::string GetControlUnit(int32_t id);

} // namepsace v4l2helper

#endif // V4L2HELPER_H

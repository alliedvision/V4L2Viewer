/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        V4L2Helper.h

Description: This namespace is used as helper. Functions are mostly used
             in the Camera class.

-------------------------------------------------------------------------------

THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#ifndef V4L2HELPER_H
#define V4L2HELPER_H

#include <stdint.h>

#include <string>

namespace v4l2helper
{

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

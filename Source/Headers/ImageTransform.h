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


#ifndef IMAGETRANSFORM_H
#define IMAGETRANSFORM_H

#include <QImage>

#include <stdint.h>

class ImageTransform
{
public:

    ImageTransform();

    virtual ~ImageTransform();

    // This function convert frame and return results of conversion
    //
    // Parameters:
    // [in] (const uint8_t *) pBuffer
    // [in] (uint32_t) length - length of the buffer
    // [in] (uint32_t) width - width of the frame
    // [in] (uint32_t) height - height of the frame
    // [in] (uint32_t) pixelFormat
    // [in] (uint32_t &) payloadSize
    // [in] (uint32_t &) bytesPerLine
    // [in] (QImage &) convertedImage
    //
    // Returns:
    // (int) - result of converting
    static int ConvertFrame(const uint8_t* pBuffer, uint32_t length,
                            uint32_t width, uint32_t height, uint32_t pixelFormat,
                            uint32_t &payloadSize, uint32_t &bytesPerLine, QImage &convertedImage);
};

#endif // IMAGETRANSFORM_H


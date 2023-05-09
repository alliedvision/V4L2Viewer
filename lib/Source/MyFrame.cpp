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


#include "MyFrame.h"

#include <string.h>

MyFrame::MyFrame(uint32_t &bufferIndex, uint8_t *&buffer, uint32_t &length,
                 uint32_t &width, uint32_t &height, uint32_t &pixelFormat,
                 uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID)
    : m_pBuffer(buffer)
    , m_FrameId(frameID)
    , m_Width(width)
    , m_Height(height)
    , m_PixelFormat(pixelFormat)
    , m_PayloadSize(payloadSize)
    , m_BytesPerLine(bytesPerLine)
    , m_Bufferlength(length)
    , m_BufferIndex(bufferIndex)
{
}

MyFrame::MyFrame(QImage &image, uint32_t &bufferIndex, uint8_t *&buffer, uint32_t &length,
                 uint32_t &width, uint32_t &height, uint32_t &v,
                 uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID)
    : m_Image(image)
    , m_pBuffer(NULL)
    , m_FrameId(frameID)
    , m_Width(width)
    , m_Height(height)
    , m_PixelFormat(0)
    , m_PayloadSize(payloadSize)
    , m_BytesPerLine(bytesPerLine)
    , m_Bufferlength(length)
    , m_BufferIndex(bufferIndex)
{
    m_Buffer.resize(length);
    memcpy(m_Buffer.data(), buffer, length);
    m_pBuffer = m_Buffer.data();
}

MyFrame::MyFrame(QImage &image, unsigned long long frameID)
    : m_Image(image)
    , m_pBuffer(NULL)
    , m_FrameId(frameID)
    , m_Width(0)
    , m_Height(0)
    , m_PixelFormat(0)
    , m_PayloadSize(0)
    , m_BytesPerLine(0)
    , m_Bufferlength(0)
    , m_BufferIndex(0)
{
}

MyFrame::MyFrame(const MyFrame *pFrame)
    : m_Image(pFrame->m_Image)
    , m_pBuffer(NULL)
    , m_FrameId(pFrame->m_FrameId)
    , m_Width(0)
    , m_Height(0)
    , m_PixelFormat(0)
    , m_PayloadSize(0)
    , m_BytesPerLine(0)
    , m_Bufferlength(0)
    , m_BufferIndex(0)
{
}

MyFrame::~MyFrame(void)
{
}

// Get the frame buffer
QImage &MyFrame::GetImage()
{
    return m_Image;
}

uint8_t *MyFrame::GetBuffer()
{
    return m_pBuffer;
}

uint32_t MyFrame::GetBufferlength()
{
    return m_Bufferlength;
}

uint32_t MyFrame::GetWidth()
{
    return m_Width;
}

uint32_t MyFrame::GetHeight()
{
    return m_Height;
}

uint32_t MyFrame::GetPixelFormat()
{
    return m_PixelFormat;
}

uint32_t MyFrame::GetPayloadSize()
{
    return m_PayloadSize;
}

uint32_t MyFrame::GetBytesPerLine()
{
    return m_BytesPerLine;
}

// get the id of the frame
unsigned long long MyFrame::GetFrameId()
{
    return m_FrameId;
}

uint32_t MyFrame::GetBufferIndex()
{
    return m_BufferIndex;
}

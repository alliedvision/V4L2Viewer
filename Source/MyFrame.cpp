/*=============================================================================
  Copyright (C) 2015 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        MyFrame.cpp

  Description: 

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

#include <string.h>
#include "MyFrame.h"

MyFrame::MyFrame(uint32_t &bufferIndex, uint8_t *&buffer, uint32_t &length, 
		 uint32_t &width, uint32_t &height, uint32_t &pixelformat,
		 uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID)
	: m_FrameId(0)
{
    m_FrameId = frameID;
    m_buffer = buffer;
    m_bufferlength = length; 
    m_Width = width;
    m_Height = height;
    m_Pixelformat = pixelformat;
    m_PayloadSize = payloadSize;
    m_BytesPerLine = bytesPerLine;
    m_bufferIndex = bufferIndex;
}

MyFrame::MyFrame(QImage &image, unsigned long long frameID) 
	: m_FrameId(0)
{
    m_FrameId = frameID;
    m_Image = image;
}

MyFrame::MyFrame(const MyFrame *pFrame) 
	: m_FrameId(0)
{
    m_FrameId = pFrame->m_FrameId;
    m_Image = pFrame->m_Image;
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
    return m_buffer;
}

uint32_t MyFrame::GetBufferlength() 
{
    return m_bufferlength;
}

uint32_t MyFrame::GetWidth()
{
    return m_Width;
}

uint32_t MyFrame::GetHeight()
{
    return m_Height;
}

uint32_t MyFrame::GetPixelformat()
{
    return m_Pixelformat;
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
    return m_bufferIndex;
}

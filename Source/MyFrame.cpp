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


MyFrame::MyFrame(const uint8_t *buffer, uint32_t length, uint32_t width, uint32_t height, uint32_t pixelformat, unsigned long long frameID, uint32_t extChunkPayloadSize) 
	: m_Format(0)
	, m_FrameId(0)
	, m_Width(0)
	, m_Height(0)
    , m_FrameCount(0)
{
	m_Width = width;
	m_Height = height;
	m_Format = pixelformat;
	m_FrameId = frameID;
    m_ExtChunkPayloadSize = extChunkPayloadSize;
	
	m_FrameBuffer.resize(length);
	memcpy(&m_FrameBuffer[0], buffer, length * sizeof(uint8_t));

    if (0 < m_ExtChunkPayloadSize)
	{
		m_ChunkDataBuffer.resize(m_ExtChunkPayloadSize);
		memcpy(&m_ChunkDataBuffer[0], (buffer + length - m_ExtChunkPayloadSize), m_ExtChunkPayloadSize * sizeof(uint8_t));
	}
}

MyFrame::MyFrame(const MyFrame *pFrame) 
	: m_Format(0)
	, m_FrameId(0)
	, m_Width(0)
	, m_Height(0)
{
	uint32_t size;

    m_Width = pFrame->m_Width;
    m_Height = pFrame->m_Height;
    m_Format = pFrame->m_Format;
    m_FrameId = pFrame->m_FrameId;
    
    size = (uint32_t)pFrame->m_FrameBuffer.size();
    
    m_FrameBuffer.resize(size);
    memcpy(&m_FrameBuffer[0], &pFrame->m_FrameBuffer[0], size * sizeof(uint8_t));

    if (0 < m_ExtChunkPayloadSize)
	{
	    size = (uint32_t)pFrame->m_ChunkDataBuffer.size();
        m_ChunkDataBuffer.resize(size);
        memcpy(&m_ChunkDataBuffer[0], &pFrame->m_ChunkDataBuffer[0], size * sizeof(uint8_t));
    }
}

MyFrame::~MyFrame(void)
{
}

// Get the frame buffer
uint8_t* MyFrame::GetFrameBuffer()
{
	return &m_FrameBuffer[0];
}

uint32_t MyFrame::GetFrameBufferLength()
{
	return (uint32_t)m_FrameBuffer.size();
}

// Get the chunk data buffer
uint8_t* MyFrame::GetChunkDataBuffer()
{
	return &m_ChunkDataBuffer[0];
}

uint32_t MyFrame::GetChunkDataBufferLength()
{
	return (uint32_t)m_ChunkDataBuffer.size();
}

// Get the pixel format of the frame
uint32_t MyFrame::GetFormat()
{
	return m_Format;
}

// get the id of the frame
unsigned long long MyFrame::GetFrameId()
{
	return m_FrameId;
}

// Get the width of the frame
uint32_t MyFrame::GetWidth()
{
	return m_Width;
}

// Get the height of the frame
uint32_t MyFrame::GetHeight()
{
	return m_Height;
}

void MyFrame::SetFrameCount(unsigned long long frameCount)
{
    m_FrameCount = frameCount;
}

unsigned long long MyFrame::GetFrameCount()
{
    return m_FrameCount;
}

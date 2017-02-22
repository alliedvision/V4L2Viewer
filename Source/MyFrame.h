/*=============================================================================
  Copyright (C) 2015 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        MyFrame.h

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

#ifndef MYFRAME_H
#define MYFRAME_H

#include <vector>
#include <stdint.h>
#include <iterator>
#include <algorithm>

class MyFrame
{
public:
	// Copy the of the given frame
    MyFrame(const MyFrame *pFrame);
	MyFrame(const uint8_t *buffer, uint32_t length, uint32_t width, uint32_t height, uint32_t pixelformat, unsigned long long frameID, uint32_t extChunkPayloadSize);
	~MyFrame(void);

	// Get the frame buffer
	uint8_t* GetFrameBuffer();
    uint32_t GetFrameBufferLength();

	// Get the chunk data buffer
	uint8_t* GetChunkDataBuffer();
    uint32_t GetChunkDataBufferLength();

	// Get the pixel format of the frame
	uint32_t		        GetFormat();

	// get the id of the frame
	unsigned long long		GetFrameId();

	// Get the width of the frame
	uint32_t				GetWidth();

	// Get the height of the frame
	uint32_t				GetHeight();

    // set the frame count
    void SetFrameCount(unsigned long long frameCount);

    // get the frame count
    unsigned long long GetFrameCount();

private:
	std::vector<uint8_t>    m_FrameBuffer;
	std::vector<uint8_t>    m_ChunkDataBuffer;
	uint32_t		        m_Format;
	unsigned long long		m_FrameId;
	uint32_t				m_Width;
	uint32_t				m_Height;
    uint32_t				m_ExtChunkPayloadSize;
    unsigned long long      m_FrameCount;
};

#endif // MYFRAME_H


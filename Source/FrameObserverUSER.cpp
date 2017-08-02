/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameObserver.cpp

  Description: The frame observer that is used for notifications
               regarding the arrival of a newly acquired frame.

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

#include <sstream>
#include <QPixmap>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "videodev2_av.h"

#include <FrameObserverUSER.h>
#include <Logger.h>

namespace AVT {
namespace Tools {
namespace Examples {


////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////    

FrameObserverUSER::FrameObserverUSER(bool showFrames)
	: FrameObserver(showFrames) 
{
}

FrameObserverUSER::~FrameObserverUSER()
{
}

int FrameObserverUSER::ReadFrame(v4l2_buffer &buf)
{
    int result = -1;

	memset(&buf, 0, sizeof(buf));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_USERPTR;
	
    if (m_bStreamRunning)
		result = V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_DQBUF, &buf);

	return result;
}

int FrameObserverUSER::GetFrameData(v4l2_buffer &buf, PUSER_BUFFER &userBuffer, uint8_t *&buffer, uint32_t &length)
{
    int result = -1;

    if (m_bStreamRunning) 
    {
	length = buf.length;
	buffer = (uint8_t*)buf.m.userptr;
		
	if (0 != buffer && 0 != length)
	{  
	    result = 0;
	}
    }
	
    return result;
}

/*********************************************************************************************************/
// Frame buffer handling
/*********************************************************************************************************/
uint32_t FrameObserverUSER::GetBufferType()
{
    return V4L2_MEMORY_USERPTR;
}

int FrameObserverUSER::CreateSingleUserBuffer(uint32_t index, uint32_t bufferSize, PUSER_BUFFER &userBuffer)
{
    int result = -1;

    userBuffer = new USER_BUFFER;
    userBuffer->nBufferlength = bufferSize;
    m_RealPayloadsize = userBuffer->nBufferlength;
    userBuffer->pBuffer = new uint8_t[bufferSize];

    if (!userBuffer->pBuffer) 
    {
	Logger::LogEx("FrameObserverUSER::CreateUserBuffer buffer creation error");
	emit OnError_Signal("FrameObserverUSER::CreateUserBuffer: buffer creation error.");
	return -1;
    }

    return result;
}

int FrameObserverUSER::QueueSingleUserBuffer(const int index, uint8_t *pBuffer, uint32_t nBufferLength)
{
	int result = 0;
	v4l2_buffer buf;
	
	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.index = index;
	buf.memory = V4L2_MEMORY_USERPTR;
        buf.m.userptr = (unsigned long)pBuffer;
        buf.length = nBufferLength;
	
	if (m_bStreamRunning)
	{  
	    if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
	    {
		Logger::LogEx("FrameObserverUSER::QueueSingleUserBuffer VIDIOC_QBUF queue #%d buffer=%p failed", index, pBuffer);
	    }
	}
				    
	return result;
}

int FrameObserverUSER::DeleteSingleUserBuffer(PUSER_BUFFER &userBuffer)
{
    int result = 0;

    return result;
}

}}} // namespace AVT::Tools::Examples

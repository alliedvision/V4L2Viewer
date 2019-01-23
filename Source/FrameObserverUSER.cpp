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
#include <stdlib.h>
#include <cstdlib>

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

int FrameObserverUSER::GetFrameData(v4l2_buffer &buf, uint8_t *&buffer, uint32_t &length)
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

int FrameObserverUSER::CreateAllUserBuffer(uint32_t bufferCount, uint32_t bufferSize)
{
    int result = -1;

    if (bufferCount <= MAX_VIEWER_USER_BUFFER_COUNT)
    {
        v4l2_requestbuffers req;

	// creates user defined buffer
	CLEAR(req);

	req.count  = bufferCount;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	// requests 4 video capture buffer. Driver is going to configure all parameter and doesn't allocate them.
	if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_REQBUFS, &req)) 
	{
	    if (EINVAL == errno) 
	    {
		Logger::LogEx("FrameObserverUSER::CreateUserBuffer VIDIOC_REQBUFS does not support user pointer i/o");
		emit OnError_Signal("FrameObserverUSER::CreateUserBuffer: VIDIOC_REQBUFS does not support user pointer i/o.");
	    } else {
		Logger::LogEx("FrameObserverUSER::CreateUserBuffer VIDIOC_REQBUFS error");
		emit OnError_Signal("FrameObserverUSER::CreateUserBuffer: VIDIOC_REQBUFS error.");
	    }
	}
	else 
	{
	    AVT::BaseTools::AutoLocalMutex guard(m_UsedBufferMutex);
	
	    Logger::LogEx("FrameObserverUSER::CreateUserBuffer VIDIOC_REQBUFS OK");
	    emit OnMessage_Signal("FrameObserverUSER::CreateUserBuffer: VIDIOC_REQBUFS OK.");
	
	    // create local buffer container
	    m_UserBufferContainerList.resize(bufferCount);

	    if (m_UserBufferContainerList.size() != bufferCount) 
	    {
		Logger::LogEx("FrameObserverUSER::CreateUserBuffer buffer container error");
		emit OnError_Signal("FrameObserverUSER::CreateUserBuffer: buffer container error.");
		return -1;
	    }

	    // get the length and start address of each of the 4 buffer structs and assign the user buffer addresses
	    for (int x = 0; x < m_UserBufferContainerList.size(); ++x) 
	    {
		PUSER_BUFFER pTmpBuffer = new USER_BUFFER;
		pTmpBuffer->nBufferlength = bufferSize;
		m_RealPayloadsize = pTmpBuffer->nBufferlength;

		// buffer needs to be aligned to 128 bytes
		if (bufferSize % 128)
			bufferSize = ((bufferSize / 128) + 1) * 128;
		pTmpBuffer->pBuffer = static_cast<uint8_t*>(aligned_alloc(128, bufferSize));

		if (!pTmpBuffer->pBuffer) 
		{
		    delete pTmpBuffer;
		    Logger::LogEx("FrameObserverUSER::CreateUserBuffer buffer creation error");
		    emit OnError_Signal("FrameObserverUSER::CreateUserBuffer: buffer creation error.");
		    m_UserBufferContainerList.resize(0);
		    return -1;
		}
		else
		    m_UserBufferContainerList[x] = pTmpBuffer;
	    }
		
	    result = 0;
	}
    }

    return result;
}

int FrameObserverUSER::QueueAllUserBuffer()
{
    int result = -1;
    AVT::BaseTools::AutoLocalMutex guard(m_UsedBufferMutex);
    
    // queue the buffer
    for (uint32_t i=0; i<m_UserBufferContainerList.size(); i++)
    {
	v4l2_buffer buf;

	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.index = i;
	buf.memory = V4L2_MEMORY_USERPTR;
	buf.m.userptr = (unsigned long)m_UserBufferContainerList[i]->pBuffer;
	buf.length = m_UserBufferContainerList[i]->nBufferlength;
		
	if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
	{
	    Logger::LogEx("Camera::QueueUserBuffer VIDIOC_QBUF queue #%d buffer=%p failed", i, m_UserBufferContainerList[i]->pBuffer);
	    return result;
	}
	else
	{
            Logger::LogEx("FrameObserverUSER::QueueUserBuffer VIDIOC_QBUF queue #%d buffer=%p OK", i, m_UserBufferContainerList[i]->pBuffer);
	    result = 0;
	}
    }
    
    return result;
}

int FrameObserverUSER::QueueSingleUserBuffer(const int index)
{
    int result = 0;
    v4l2_buffer buf;
    AVT::BaseTools::AutoLocalMutex guard(m_UsedBufferMutex);
    
    if (index < m_UserBufferContainerList.size())
    {
	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.index = index;
	buf.memory = V4L2_MEMORY_USERPTR;
	buf.m.userptr = (unsigned long)m_UserBufferContainerList[index]->pBuffer;
	buf.length = m_UserBufferContainerList[index]->nBufferlength;
	
	if (m_bStreamRunning)
	{  
	    if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
	    {
		Logger::LogEx("FrameObserverUSER::QueueSingleUserBuffer VIDIOC_QBUF queue #%d buffer=%p failed", index, m_UserBufferContainerList[index]->pBuffer);
	    }
	}
    }
				    
    return result;
}

int FrameObserverUSER::DeleteAllUserBuffer()
{
    int result = 0;

    // free all internal buffers
    v4l2_requestbuffers req;
    // creates user defined buffer
    CLEAR(req);
    req.count  = 0;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;
    
    // requests 0 video capture buffer. Driver is going to configure all parameter and frees them.
    V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_REQBUFS, &req);
	
    {
	AVT::BaseTools::AutoLocalMutex guard(m_UsedBufferMutex);
	
	// delete all user buffer
	for (int x = 0; x < m_UserBufferContainerList.size(); x++)
	{
	    if (0 != m_UserBufferContainerList[x]->pBuffer)
		delete [] m_UserBufferContainerList[x]->pBuffer;
	    if (0 != m_UserBufferContainerList[x])
		delete m_UserBufferContainerList[x];
	}
    
	m_UserBufferContainerList.resize(0);
    }
	
    return result;
}

}}} // namespace AVT::Tools::Examples

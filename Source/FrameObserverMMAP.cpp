/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameObserverMMAP.cpp

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

#include <FrameObserverMMAP.h>
#include <Logger.h>

namespace AVT {
namespace Tools {
namespace Examples {


////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////    

FrameObserverMMAP::FrameObserverMMAP(bool showFrames) 
	: FrameObserver(showFrames)
{
}

FrameObserverMMAP::~FrameObserverMMAP()
{
}

int FrameObserverMMAP::ReadFrame(v4l2_buffer &buf)
{
    int result = -1;

	memset(&buf, 0, sizeof(buf));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
	
    if (m_bStreamRunning)
		result = V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_DQBUF, &buf);
	
	return result;
}

int FrameObserverMMAP::GetFrameData(v4l2_buffer &buf, uint8_t *&buffer, uint32_t &length)
{
    int result = -1;

	if (m_bStreamRunning) 
	{
	    AVT::BaseTools::AutoLocalMutex guard(m_UsedBufferMutex);
	    
	    if (buf.index < m_UserBufferContainerList.size())
	    {
		length = m_UserBufferContainerList[buf.index]->nBufferlength;
		buffer = m_UserBufferContainerList[buf.index]->pBuffer;
	    }
	    else
	    {
		length = 0;
		buffer = 0;
	    }
		
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

int FrameObserverMMAP::CreateAllUserBuffer(uint32_t bufferCount, uint32_t bufferSize)
{
    int result = -1;

    if (bufferCount <= MAX_VIEWER_USER_BUFFER_COUNT)
    {
        v4l2_requestbuffers req;

	// creates user defined buffer
	CLEAR(req);

        req.count  = bufferCount;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	// requests 4 video capture buffer. Driver is going to configure all parameter and doesn't allocate them.
	if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_REQBUFS, &req)) 
	{
	    if (EINVAL == errno) 
	    {
		Logger::LogEx("FrameObserverMMAP::CreateUserBuffer VIDIOC_REQBUFS does not support user pointer i/o");
		emit OnError_Signal("FrameObserverMMAP::CreateUserBuffer: VIDIOC_REQBUFS does not support user pointer i/o.");
	    } else {
		Logger::LogEx("FrameObserverMMAP::CreateUserBuffer VIDIOC_REQBUFS error");
		emit OnError_Signal("FrameObserverMMAP::CreateUserBuffer: VIDIOC_REQBUFS error.");
	    }
	}
	else 
	{
	    AVT::BaseTools::AutoLocalMutex guard(m_UsedBufferMutex);
		    
	    Logger::LogEx("FrameObserverMMAP::CreateUserBuffer VIDIOC_REQBUFS OK");
	    emit OnMessage_Signal("FrameObserverMMAP::CreateUserBuffer: VIDIOC_REQBUFS OK.");
		
	    // create local buffer container
	    m_UserBufferContainerList.resize(bufferCount);
        
	    if (m_UserBufferContainerList.size() != bufferCount) 
	    {
		Logger::LogEx("FrameObserverMMAP::CreateUserBuffer buffer container error");
		emit OnError_Signal("FrameObserverMMAP::CreateUserBuffer: buffer container error.");
		return -1;
	    }

	    for (int x = 0; x < bufferCount; ++x)
	    {
	    	v4l2_buffer buf;
	    	CLEAR(buf);
	    	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    	buf.memory = V4L2_MEMORY_MMAP;
	    	buf.index = x;

		if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYBUF, &buf)) 
		{
		    Logger::LogEx("FrameObserverMMAP::CreateUserBuffer VIDIOC_QUERYBUF error");
		    emit OnError_Signal("FrameObserverMMAP::CreateUserBuffer: VIDIOC_QUERYBUF error.");
		    return -1;
		}
	
		Logger::LogEx("FrameObserverMMAP::CreateUserBuffer VIDIOC_QUERYBUF MMAP OK length=%d", buf.length);
		emit OnMessage_Signal(QString("FrameObserverMMAP::CreateUserBuffer: VIDIOC_QUERYBUF OK length=%1.").arg(buf.length));
				
		PUSER_BUFFER pTmpBuffer = new USER_BUFFER;
		pTmpBuffer = new USER_BUFFER;
		pTmpBuffer->nBufferlength = buf.length;
		m_RealPayloadsize = pTmpBuffer->nBufferlength;
		pTmpBuffer->pBuffer = (uint8_t*)mmap(NULL,
						    buf.length,
						    PROT_READ | PROT_WRITE,
						    MAP_SHARED,
						    m_nFileDescriptor,
						    buf.m.offset);
	
		if (MAP_FAILED == pTmpBuffer->pBuffer)
		{
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

int FrameObserverMMAP::QueueAllUserBuffer()
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
	buf.memory = V4L2_MEMORY_MMAP;
	        
	if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
	{
	    Logger::LogEx("FrameObserverMMAP::QueueUserBuffer VIDIOC_QBUF queue #%d buffer=%p failed", i, m_UserBufferContainerList[i]->pBuffer);
	    return result;
	}
	else
	{
            Logger::LogEx("FrameObserverMMAP::QueueUserBuffer VIDIOC_QBUF queue #%d buffer=%p OK", i, m_UserBufferContainerList[i]->pBuffer);
	    result = 0;
	}
    }
    
    return result;
}

int FrameObserverMMAP::QueueSingleUserBuffer(const int index)
{
    int result = 0;
    v4l2_buffer buf;
    AVT::BaseTools::AutoLocalMutex guard(m_UsedBufferMutex);
    
    if (index < m_UserBufferContainerList.size())
    {
	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.index = index;
	buf.memory = V4L2_MEMORY_MMAP;
	
	if (m_bStreamRunning)
	{  
	    if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
	    {
		Logger::LogEx("FrameObserverMMAP::QueueSingleUserBuffer VIDIOC_QBUF queue #%d buffer=%p failed", index, m_UserBufferContainerList[index]->pBuffer);
	    }
	}
    }
				    
    return result;
}

int FrameObserverMMAP::DeleteAllUserBuffer()
{
    int result = 0;

    // free all internal buffers
    v4l2_requestbuffers req;
    // creates user defined buffer
    CLEAR(req);
    req.count  = 0;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    // requests 4 video capture buffer. Driver is going to configure all parameter and doesn't allocate them.
    V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_REQBUFS, &req);
	
    {
	AVT::BaseTools::AutoLocalMutex guard(m_UsedBufferMutex);
	
	// delete all user buffer
	for (int x = 0; x < m_UserBufferContainerList.size(); x++)
	{
	    munmap(m_UserBufferContainerList[x]->pBuffer, m_UserBufferContainerList[x]->nBufferlength);
	      
	    if (0 != m_UserBufferContainerList[x])
		delete m_UserBufferContainerList[x];
	}
    	
	m_UserBufferContainerList.resize(0);
    }
	
    return result;
}

}}} // namespace AVT::Tools::Examples

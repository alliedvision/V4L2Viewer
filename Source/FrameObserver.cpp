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
#include <linux/videodev2.h>

#include <FrameObserver.h>
#include <Logger.h>

#define CLIP(color) (unsigned char)(((color) > 0xFF) ? 0xff : (((color) < 0) ? 0 : (color)))

namespace AVT {
namespace Tools {
namespace Examples {


////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////    

FrameObserver::FrameObserver() 
	: m_nReceivedFramesCounter(0)
	, m_nIncompletedFramesCounter(0)
	, m_bTerminate(false)
	, m_bRecording(false)
	, m_bAbort(false)
	, m_nFileDescriptor(0)
	, m_nWidth(0)
	, m_nHeight(0)
	, m_FrameId(0)
	, m_PayloadSize(0)
	, m_BytesPerLine(0)
	, m_pBuffer(0)
	, m_MessageSendFlag(false)
        , m_BlockingMode(false)
	, m_UseMMAPBuffer(false)
        , m_UsedBufferCount(0)
	, m_bStreamRunning(false)
	, m_bStreamStopped(false)
{
	start();
}

FrameObserver::~FrameObserver()
{
	// stop the internal processing thread and wait until the thread is really stopped
	m_bAbort = true;

	// wait until the thread is stopped
	while (isRunning())
		QThread::msleep(10);
}

int FrameObserver::StartStream(bool blockingMode, int fileDescriptor, uint32_t pixelformat, uint32_t payloadsize, uint32_t width, uint32_t height, uint32_t bytesPerLine)
{
    int nResult = 0;
    
    m_BlockingMode = blockingMode;
    m_nFileDescriptor = fileDescriptor;
    m_nWidth = width;
    m_nHeight = height;
    m_FrameId = 0;
    m_nReceivedFramesCounter = 0;
    m_PayloadSize = payloadsize;
    m_Pixelformat = pixelformat;
    m_BytesPerLine = bytesPerLine;
    m_pBuffer = new uint8_t[payloadsize];
    m_MessageSendFlag = false;
    
    m_bStreamStopped = false;
    m_bStreamRunning = true;
    
    m_UserBufferContainerList.resize(0);
    
    return nResult;
}

int FrameObserver::StopStream()
{
    int nResult = 0;
    
    m_bStreamRunning = false;
    
    while (!m_bStreamStopped)
         QThread::msleep(10);

    return nResult;
}

void FrameObserver::SetTerminateFlag()// Event will be called when the frame processing is done and the frame can be returned to streaming engine
{
    m_bTerminate = true;
}

void v4lconvert_uyvy_to_rgb24(const unsigned char *src, unsigned char *dest,
		int width, int height, int stride)
{
	int j;

	while (--height >= 0) {
		for (j = 0; j + 1 < width; j += 2) {
			int u = src[0];
			int v = src[2];
			int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
			int rg = (((u - 128) << 1) +  (u - 128) +
					((v - 128) << 2) + ((v - 128) << 1)) >> 3;
			int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

			*dest++ = CLIP(src[1] + v1);
			*dest++ = CLIP(src[1] - rg);
			*dest++ = CLIP(src[1] + u1);

			*dest++ = CLIP(src[3] + v1);
			*dest++ = CLIP(src[3] - rg);
			*dest++ = CLIP(src[3] + u1);
			src += 4;
		}
		src += stride - width * 2;
	}
}

void v4lconvert_yuyv_to_rgb24(const unsigned char *src, unsigned char *dest,
		int width, int height, int stride)
{
	int j;

	while (--height >= 0) {
		for (j = 0; j + 1 < width; j += 2) {
			int u = src[1];
			int v = src[3];
			int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
			int rg = (((u - 128) << 1) +  (u - 128) +
					((v - 128) << 2) + ((v - 128) << 1)) >> 3;
			int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

			*dest++ = CLIP(src[0] + v1);
			*dest++ = CLIP(src[0] - rg);
			*dest++ = CLIP(src[0] + u1);

			*dest++ = CLIP(src[2] + v1);
			*dest++ = CLIP(src[2] - rg);
			*dest++ = CLIP(src[2] + u1);
			src += 4;
		}
		src += stride - (width * 2);
	}
}

void v4lconvert_yuv420_to_rgb24(const unsigned char *src, unsigned char *dest,
		int width, int height, int yvu)
{
	int i, j;

	const unsigned char *ysrc = src;
	const unsigned char *usrc, *vsrc;

	if (yvu) {
		vsrc = src + width * height;
		usrc = vsrc + (width * height) / 4;
	} else {
		usrc = src + width * height;
		vsrc = usrc + (width * height) / 4;
	}

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j += 2) {
#if 1 /* fast slightly less accurate multiplication free code */
			int u1 = (((*usrc - 128) << 7) +  (*usrc - 128)) >> 6;
			int rg = (((*usrc - 128) << 1) +  (*usrc - 128) +
					((*vsrc - 128) << 2) + ((*vsrc - 128) << 1)) >> 3;
			int v1 = (((*vsrc - 128) << 1) +  (*vsrc - 128)) >> 1;

			*dest++ = CLIP(*ysrc + v1);
			*dest++ = CLIP(*ysrc - rg);
			*dest++ = CLIP(*ysrc + u1);
			ysrc++;

			*dest++ = CLIP(*ysrc + v1);
			*dest++ = CLIP(*ysrc - rg);
			*dest++ = CLIP(*ysrc + u1);
#else
			*dest++ = YUV2R(*ysrc, *usrc, *vsrc);
			*dest++ = YUV2G(*ysrc, *usrc, *vsrc);
			*dest++ = YUV2B(*ysrc, *usrc, *vsrc);
			ysrc++;

			*dest++ = YUV2R(*ysrc, *usrc, *vsrc);
			*dest++ = YUV2G(*ysrc, *usrc, *vsrc);
			*dest++ = YUV2B(*ysrc, *usrc, *vsrc);
#endif
			ysrc++;
			usrc++;
			vsrc++;
		}
		/* Rewind u and v for next line */
		if (!(i&1)) {
			usrc -= width / 2;
			vsrc -= width / 2;
		}
	}
}

int FrameObserver::DisplayFrame(const uint8_t* pBuffer, uint32_t length)
{
	int result = 0;
	QImage convertedImage;
	
	if (NULL == pBuffer || 0 == length)
	    return -1;
	
	if (m_Pixelformat == V4L2_PIX_FMT_JPEG ||
		m_Pixelformat == V4L2_PIX_FMT_MJPEG)
	{	
		QPixmap pix;
		pix.loadFromData(pBuffer, m_PayloadSize, "JPG");
		convertedImage = pix.toImage();
	}
	else if (m_Pixelformat == V4L2_PIX_FMT_UYVY)
	{	
		convertedImage = QImage(m_nWidth, m_nHeight, QImage::Format_RGB888);
		v4lconvert_uyvy_to_rgb24(pBuffer, convertedImage.bits(), m_nWidth, m_nHeight, m_BytesPerLine);
	}
	else if (m_Pixelformat == V4L2_PIX_FMT_YUYV)
	{	
		convertedImage = QImage(m_nWidth, m_nHeight, QImage::Format_RGB888);
		v4lconvert_yuyv_to_rgb24(pBuffer, convertedImage.bits(), m_nWidth, m_nHeight, m_BytesPerLine);
	}
	else if (m_Pixelformat == V4L2_PIX_FMT_YUV420)
	{	
		convertedImage = QImage(m_nWidth, m_nHeight, QImage::Format_RGB888);
	        v4lconvert_yuv420_to_rgb24(pBuffer, convertedImage.bits(), m_nWidth, m_nHeight, 1);
	}	
	else if (m_Pixelformat == V4L2_PIX_FMT_RGB24 ||
			 m_Pixelformat == V4L2_PIX_FMT_BGR24)
	{	
		convertedImage = QImage(pBuffer, m_nWidth, m_nHeight, QImage::Format_RGB888);
	}
	else if (m_Pixelformat == V4L2_PIX_FMT_RGB32 ||
			 m_Pixelformat == V4L2_PIX_FMT_BGR32)
	{
		convertedImage = QImage(pBuffer, m_nWidth, m_nHeight, QImage::Format_RGB32);
	}
	else
	{
		return -1;
	}
	
	//QImage convertedImage((uint8_t*)m_pBuffer, m_nWidth, m_nHeight, QImage::Format_RGB32);
	
	emit OnFrameReady_Signal(convertedImage, m_FrameId);
	
	return result;
}

int FrameObserver::ReadFrame()
{
    v4l2_buffer buf;
    int result = -1;

	memset(&buf, 0, sizeof(buf));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (m_UseMMAPBuffer)
	  buf.memory = V4L2_MEMORY_MMAP;
	else
	  buf.memory = V4L2_MEMORY_USERPTR;
	
	result = xioctl(m_nFileDescriptor, VIDIOC_DQBUF, &buf);

	if (result >= 0  && m_bStreamRunning) 
	{
		uint32_t length = buf.length;
		uint8_t* buffer = (uint8_t*)buf.m.userptr;
		
		if (m_UseMMAPBuffer)
		{
		    length = m_UserBufferContainerList[buf.index]->nBufferlength;
		    buffer = m_UserBufferContainerList[buf.index]->pBuffer;
		}  
		if (0 != buffer && 0 != length)
		{  
		    m_FrameId++;
		    m_nReceivedFramesCounter++;
		    
		    result = DisplayFrame(buffer, length);
		
		    if (m_bRecording && -1 != result)
		    {
			if (m_FrameRecordQueue.GetSize() < MAX_FRAME_QUEUE_SIZE)
			{
				m_FrameRecordQueue.Enqueue(buffer, length, m_nWidth, m_nHeight, m_Pixelformat, m_FrameId, 0, m_FrameId);
				OnRecordFrame_Signal(m_FrameId, m_FrameRecordQueue.GetSize());
			}
			else
			{
				if (m_FrameRecordQueue.GetSize() == MAX_FRAME_QUEUE_SIZE)
					OnMessage_Signal(QString("Following frames are not saved, more than %1 would freeze the system.").arg(MAX_FRAME_QUEUE_SIZE));
			}
		    }
		
		    
		
		    QueueSingleUserBuffer(buf.index);
		    //FrameDone((uint64_t)buf.m.userptr);
		}
		else
		{
		    if (!m_MessageSendFlag)
		    {
		        m_MessageSendFlag = true;
		        OnMessage_Signal(QString("Frame buffer empty !"));
		    }
		    
		    if (m_bStreamRunning)
			xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf);
		}
	}
	if (!m_bStreamRunning)
	    m_bStreamStopped = true;

	return result;
}

// Do the work within this thread
void FrameObserver::run()
{
        OnMessage_Signal(QString("FrameObserver thread started."));
	while (!m_bAbort)
	{
		fd_set fds;
		struct timeval tv;
		int result = -1;

		FD_ZERO(&fds);
		FD_SET(m_nFileDescriptor, &fds);

                if (!m_BlockingMode)
                {
		    /* Timeout. */
		    tv.tv_sec = 10;
		    tv.tv_usec = 0;

		    result = select(m_nFileDescriptor + 1, &fds, NULL, NULL, &tv);

		    if (-1 == result) 
		    {
			    // Error
			    continue;
		    } else if (0 == result) 
		    {
			    // Timeout
			    QThread::msleep(0);
			    continue;
		    } else 
		    {
			    ReadFrame();
		    }
                 }
                 else
                 {
                     ReadFrame();
                 }
	}
	OnMessage_Signal(QString("FrameObserver thread stopped."));
}

int FrameObserver::xioctl(int fh, int request, void *arg)
{
	int result = 0;

    do 
	{
		result = ioctl(fh, request, arg);
    } 
	while (-1 == result && EINTR == errno);

    return result;
}

// Get the number of frames
// This function will clear the counter of received frames
unsigned int FrameObserver::GetReceivedFramesCount()
{
	unsigned int res = m_nReceivedFramesCounter;

	m_nReceivedFramesCounter = 0;

	return res;
}

// Get the number of uncompleted frames
unsigned int FrameObserver::GetIncompletedFramesCount()
{
	return 0;
}

// Set the number of uncompleted frames
void FrameObserver::ResetIncompletedFramesCount()
{
	m_nIncompletedFramesCounter = 0;
}


// Recording

void FrameObserver::SetRecording(bool start)
{
    m_bRecording = start;
}

void FrameObserver::DisplayStepBack()
{
    QSharedPointer<MyFrame> pFrame = m_FrameRecordQueue.GetPrevious();

    if (NULL != pFrame)
    {
        uint64_t frameID = pFrame->GetFrameId();
        uint32_t width = pFrame->GetWidth();
        uint32_t height = pFrame->GetHeight();
        uint32_t pixelformat = pFrame->GetFormat();
        OnDisplayFrame_Signal(frameID, width, height, pixelformat);

		DisplayFrame(pFrame->GetFrameBuffer(), pFrame->GetFrameBufferLength());
    }
}

void FrameObserver::DisplayStepForw()
{
    QSharedPointer<MyFrame> pFrame = m_FrameRecordQueue.GetNext();

    if (NULL != pFrame)
    {
        uint64_t frameID = pFrame->GetFrameId();
        uint32_t width = pFrame->GetWidth();
        uint32_t height = pFrame->GetHeight();
        uint32_t pixelformat = pFrame->GetFormat();
        OnDisplayFrame_Signal(frameID, width, height, pixelformat);
		
		DisplayFrame(pFrame->GetFrameBuffer(), pFrame->GetFrameBufferLength());
    }
}

void FrameObserver::DeleteRecording()
{
    m_FrameRecordQueue.Clear();
}



/*********************************************************************************************************/
// Frame buffer handling
/*********************************************************************************************************/

int FrameObserver::CreateUserBuffer(uint32_t bufferCount, uint32_t bufferSize, bool internalBuffer)
{
    int result = -1;

    m_UseMMAPBuffer = internalBuffer;

    if (bufferCount <= MAX_VIEWER_USER_BUFFER_COUNT)
    {
        v4l2_requestbuffers req;

		// creates user defined buffer
		CLEAR(req);

        req.count  = bufferCount;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (!internalBuffer)
	{
	    req.memory = V4L2_MEMORY_USERPTR;
	} else {
	    req.memory = V4L2_MEMORY_MMAP;
	}

		// requests 4 video capture buffer. Driver is going to configure all parameter and doesn't allocate them.
		if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_REQBUFS, &req)) 
		{
			if (EINVAL == errno) 
			{
				Logger::LogEx("Camera::CreateUserBuffer VIDIOC_REQBUFS does not support user pointer i/o");
				emit OnError_Signal("Camera::CreateUserBuffer: VIDIOC_REQBUFS does not support user pointer i/o.");
			} else {
				Logger::LogEx("Camera::CreateUserBuffer VIDIOC_REQBUFS error");
				emit OnError_Signal("Camera::CreateUserBuffer: VIDIOC_REQBUFS error.");
			}
		}
		else 
		{
			Logger::LogEx("Camera::CreateUserBuffer VIDIOC_REQBUFS OK");
			emit OnMessage_Signal("Camera::CreateUserBuffer: VIDIOC_REQBUFS OK.");
		
			// create local buffer container
			m_UserBufferContainerList.resize(bufferCount);
        
			if (m_UserBufferContainerList.size() != bufferCount) 
		        {
			    Logger::LogEx("Camera::CreateUserBuffer buffer container error");
			    emit OnError_Signal("Camera::CreateUserBuffer: buffer container error.");
			    return -1;
			}

			if (!internalBuffer)
			{
			    // get the length and start address of each of the 4 buffer structs and assign the user buffer addresses
			    for (int x = 0; x < bufferCount; ++x) 
			    {
			        m_UserBufferContainerList[x] = new USER_BUFFER;
				m_UserBufferContainerList[x]->nBufferlength = bufferSize;
				m_UserBufferContainerList[x]->pBuffer = new uint8_t[bufferSize];

				if (!m_UserBufferContainerList[x]->pBuffer) 
				{
				    Logger::LogEx("Camera::CreateUserBuffer buffer creation error");
				    emit OnError_Signal("Camera::CreateUserBuffer: buffer creation error.");
				    return -1;
				}
			    }
			}
			else
			{
			    for (int x = 0; x < bufferCount; ++x)
			    {
			    	v4l2_buffer buf;
			    	CLEAR(buf);
			    	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			    	buf.memory = V4L2_MEMORY_MMAP;
			    	buf.index = x;

				if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYBUF, &buf)) 
				{
				    Logger::LogEx("Camera::CreateUserBuffer VIDIOC_QUERYBUF error");
				    emit OnError_Signal("Camera::CreateUserBuffer: VIDIOC_QUERYBUF error.");
				    return -1;
				}

				Logger::LogEx("Camera::CreateUserBuffer VIDIOC_QUERYBUF MMAP OK length=%d", buf.length);
				emit OnError_Signal(QString("Camera::CreateUserBuffer: VIDIOC_QUERYBUF OK length=%1.").arg(buf.length));
				    
				m_UserBufferContainerList[x] = new USER_BUFFER;
				m_UserBufferContainerList[x]->nBufferlength = buf.length;
				m_UserBufferContainerList[x]->pBuffer = (uint8_t*)mmap(NULL,
									     buf.length,
									     PROT_READ | PROT_WRITE,
									     MAP_SHARED,
									     m_nFileDescriptor,
									     buf.m.offset);

				if (MAP_FAILED == m_UserBufferContainerList[x]->pBuffer)
				    return -1;
			    }
			}

			m_UsedBufferCount = bufferCount;
			result = 0;
		}
    }

    return result;
}

int FrameObserver::QueueAllUserBuffer()
{
    int result = -1;
    
    // queue the buffer
    for (uint32_t i=0; i<m_UsedBufferCount; i++)
    {
		v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.index = i;
		if (!m_UseMMAPBuffer)
		{  
		    buf.memory = V4L2_MEMORY_USERPTR;
		    buf.m.userptr = (unsigned long)m_UserBufferContainerList[i]->pBuffer;
		    buf.length = m_UserBufferContainerList[i]->nBufferlength;
		}
		else
		{
		    buf.memory = V4L2_MEMORY_MMAP;
	        }
	        
		if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
		{
			Logger::LogEx("Camera::QueueUserBuffer VIDIOC_QBUF queue #%d buffer=%p failed", i, m_UserBufferContainerList[i]->pBuffer);
			return result;
		}
		else
		{
            Logger::LogEx("Camera::QueueUserBuffer VIDIOC_QBUF queue #%d buffer=%p OK", i, m_UserBufferContainerList[i]->pBuffer);
			result = 0;
		}
    }
    
    return result;
}

int FrameObserver::QueueSingleUserBuffer(const int index)
{
	int result = 0;
	v4l2_buffer buf;
	
	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.index = index;
	if (!m_UseMMAPBuffer)
	{  
	    buf.memory = V4L2_MEMORY_USERPTR;
	    buf.m.userptr = (unsigned long)m_UserBufferContainerList[index]->pBuffer;
	    buf.length = m_UserBufferContainerList[index]->nBufferlength;
	}
	else
	{
	    buf.memory = V4L2_MEMORY_MMAP;
	}
	
	if (m_bStreamRunning)
	{  
	    if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
	    {
		Logger::LogEx("Camera::QueueSingleUserBuffer VIDIOC_QBUF queue #%d buffer=%p failed", index, m_UserBufferContainerList[index]->pBuffer);
	    }
	}
				    
	return result;
}

int FrameObserver::DeleteUserBuffer()
{
    int result = 0;

    // delete all user buffer
	for (int x = 0; x < m_UsedBufferCount; x++)
	{
	    if (!m_UseMMAPBuffer)
	    {	
	    	if (0 != m_UserBufferContainerList[x]->pBuffer)
		    delete [] m_UserBufferContainerList[x]->pBuffer;
	    }
	    else
	    {
	        munmap(m_UserBufferContainerList[x]->pBuffer, m_UserBufferContainerList[x]->nBufferlength);
	    }  
    	    if (0 != m_UserBufferContainerList[x])
		delete m_UserBufferContainerList[x];
	}
	
	m_UserBufferContainerList.resize(0);
	
	
	// free all internal buffers
	v4l2_requestbuffers req;
	// creates user defined buffer
	CLEAR(req);
        req.count  = 0;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (!m_UseMMAPBuffer) {
	    req.memory = V4L2_MEMORY_USERPTR;
	} else {
	    req.memory = V4L2_MEMORY_MMAP;
	}

	// requests 4 video capture buffer. Driver is going to configure all parameter and doesn't allocate them.
	V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_REQBUFS, &req);
	
	
    return result;
}

// The event handler to return the frame
void FrameObserver::FrameDone(const unsigned long long frameHandle)
{
    if (m_bStreamRunning)
    {
		for (int i = 0; i < m_UsedBufferCount; ++i)
		{
			if (frameHandle == (uint64_t)m_UserBufferContainerList[i]->pBuffer)
			{
                                QueueSingleUserBuffer(i);
				
				break;
			}
		}
        		
        
    }
}

}}} // namespace AVT::Tools::Examples

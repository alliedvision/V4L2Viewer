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

#include "FrameObserver.h"
#include "Logger.h"

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
	, m_MessageSendFlag(false)
        , m_BlockingMode(false)
        , m_UsedBufferCount(0)
	, m_bStreamRunning(false)
	, m_bStreamStopped(false)
{
	start();

	m_pImageProcessingThread = QSharedPointer<ImageProcessingThread>(new ImageProcessingThread());

	connect(m_pImageProcessingThread.data(), SIGNAL(OnFrameReady_Signal(const QImage &, const unsigned long long &)), this, SLOT(OnFrameReadyFromThread(const QImage &, const unsigned long long &)));

	m_pImageProcessingThread->start();
}

FrameObserver::~FrameObserver()
{
	m_pImageProcessingThread->StopThread();

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

int FrameObserver::DisplayFrame(const uint8_t* pBuffer, uint32_t length,
                                QImage &convertedImage)
{
	int result = 0;
	
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
	
	return result;
}

int FrameObserver::ReadFrame()
{
    int result = -1;

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
		    tv.tv_sec = 1;
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

void FrameObserver::OnFrameReadyFromThread(const QImage &image, const unsigned long long &frameId)
{
    emit OnFrameReady_Signal(image, frameId);
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
        OnDisplayFrame_Signal(frameID);

	emit OnFrameReady_Signal(pFrame->GetImage(), frameID);
    }
}

void FrameObserver::DisplayStepForw()
{
    QSharedPointer<MyFrame> pFrame = m_FrameRecordQueue.GetNext();

    if (NULL != pFrame)
    {
        uint64_t frameID = pFrame->GetFrameId();
        OnDisplayFrame_Signal(frameID);
		
	emit OnFrameReady_Signal(pFrame->GetImage(), frameID);
    }
}

void FrameObserver::DeleteRecording()
{
    m_FrameRecordQueue.Clear();
}



/*********************************************************************************************************/
// Frame buffer handling
/*********************************************************************************************************/

int FrameObserver::CreateUserBuffer(uint32_t bufferCount, uint32_t bufferSize)
{
    int result = -1;

    return result;
}

int FrameObserver::QueueAllUserBuffer()
{
    int result = -1;
    
    return result;
}

int FrameObserver::QueueSingleUserBuffer(const int index)
{
    int result = 0;
	
    return result;
}

int FrameObserver::DeleteUserBuffer()
{
    int result = 0;

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

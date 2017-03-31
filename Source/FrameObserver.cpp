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

FrameObserver::FrameObserver(bool showFrames) 
	: m_nReceivedFramesCounter(0)
	, m_nDroppedFramesCounter(0)
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
        , m_ShowFrames(showFrames)
{
	m_pImageProcessingThread = QSharedPointer<ImageProcessingThread>(new ImageProcessingThread());

	connect(m_pImageProcessingThread.data(), SIGNAL(OnFrameReady_Signal(const QImage &, const unsigned long long &, const int &)), this, SLOT(OnFrameReadyFromThread(const QImage &, const unsigned long long &, const int &)));

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
    
	start();

    m_UserBufferContainerList.resize(0);
    
    return nResult;
}

int FrameObserver::StopStream()
{
    int nResult = 0;
    int count = 300;
    
    m_bStreamRunning = false;
    
    while (!m_bStreamStopped && count-- > 0)
         QThread::msleep(10);

    if (count <= 0)
       nResult = -1;

    return nResult;
}

void FrameObserver::SetTerminateFlag()// Event will be called when the frame processing is done and the frame can be returned to streaming engine
{
    m_bTerminate = true;
}

int FrameObserver::ReadFrame(v4l2_buffer &buf)
{
    int result = -1;

    return result;
}

int FrameObserver::GetFrameData(v4l2_buffer &buf, uint8_t *&buffer, uint32_t &length)
{
    int result = -1;

    return result;
}

void FrameObserver::DequeueAndProcessFrame()
{
	v4l2_buffer buf;
				
	if (0 == ReadFrame(buf))
	{
		m_FrameId++;
		m_nReceivedFramesCounter++;
		
		if (m_ShowFrames)
		{
			uint8_t *buffer = 0;
			uint32_t length = 0;
			
			if (0 == GetFrameData(buf, buffer, length))
			{
				if (m_pImageProcessingThread->QueueFrame(buf, buffer, length, 
										m_nWidth, m_nHeight, m_Pixelformat, 
										m_PayloadSize, m_BytesPerLine, m_FrameId))
					m_nDroppedFramesCounter++;
			}
		}
		else
		{
			emit OnFrameID_Signal(m_FrameId);
			QueueSingleUserBuffer(buf.index);
		}
	}
}

// Do the work within this thread
void FrameObserver::run()
{
    emit OnMessage_Signal(QString("FrameObserver thread started."));
	
	m_bStreamRunning = true;
	
	while (m_bStreamRunning)
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
		    } else {
				DequeueAndProcessFrame();
		    }
		}
		else
		{
			DequeueAndProcessFrame();
		}
	}

	m_bStreamStopped = true;
	
	emit OnMessage_Signal(QString("FrameObserver thread stopped."));
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
unsigned int FrameObserver::GetDroppedFramesCount()
{
	return m_nDroppedFramesCounter;
}

// Set the number of uncompleted frames
void FrameObserver::ResetDroppedFramesCount()
{
	m_nDroppedFramesCounter = 0;
}

void FrameObserver::OnFrameReadyFromThread(const QImage &image, const unsigned long long &frameId, const int &bufIndex)
{
    emit OnFrameReady_Signal(image, frameId);
	
	QueueSingleUserBuffer(bufIndex);
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
        emit OnDisplayFrame_Signal(frameID);

	emit OnFrameReady_Signal(pFrame->GetImage(), frameID);
    }
}

void FrameObserver::DisplayStepForw()
{
    QSharedPointer<MyFrame> pFrame = m_FrameRecordQueue.GetNext();

    if (NULL != pFrame)
    {
        uint64_t frameID = pFrame->GetFrameId();
        emit OnDisplayFrame_Signal(frameID);
		
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

void FrameObserver::SwitchFrameTransfer2GUI(bool showFrames)
{
    m_ShowFrames = showFrames;
}

}}} // namespace AVT::Tools::Examples

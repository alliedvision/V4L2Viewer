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

#include "FrameObserver.h"
#include "Logger.h"
#include "ImageTransf.h"

#define CLIP(color) (unsigned char)(((color) > 0xFF) ? 0xff : (((color) < 0) ? 0 : (color)))

////////////////////////////////////////////////////////////////////////////
// Global data
////////////////////////////////////////////////////////////////////////////    

uint8_t *g_ConversionBuffer1 = 0;
uint8_t *g_ConversionBuffer2 = 0;

namespace AVT {
namespace Tools {
namespace Examples {

////////////////////////////////////////////////////////////////////////////
// C service
////////////////////////////////////////////////////////////////////////////    

uint32_t InternalConvertRAW10inRAW16ToRAW10g(const void *sourceBuffer, uint32_t length, const void *destBuffer)
{
    uint8_t *destdata = (unsigned char *)destBuffer;
    uint8_t *srcdata = (unsigned char *)sourceBuffer;
    uint32_t srcCount = 0;
	uint32_t destCount = 0;
    
    while (srcCount < length)
    {
       unsigned char lsbits = 0;

       for(int iii=0; iii<4; iii++)
       {
		  *destdata++ = *(srcdata+1); // bits [9:2]
		  destCount++;

          lsbits |= (*srcdata >> 6) << (iii*2); // least significant bits [1:0]

          srcdata += 2; // move to next 16 bits
          
          srcCount += 2;
       }

       *destdata++ = lsbits; // every 5th byte contains the lsbs from the last 4 pixels
       destCount++;
    }
    
    return destCount;
}

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////    

FrameObserver::FrameObserver(bool showFrames) 
	: m_nReceivedFramesCounter(0)
	, m_nDroppedFramesCounter(0)
	, m_bRecording(false)
	, m_nFileDescriptor(0)
	, m_nWidth(0)
	, m_nHeight(0)
	, m_FrameId(0)
	, m_PayloadSize(0)
	, m_BytesPerLine(0)
	, m_MessageSendFlag(false)
        , m_BlockingMode(false)
        , m_bStreamRunning(false)
	, m_bStreamStopped(true)
        , m_ShowFrames(showFrames)
	, m_RealPayloadsize(0)
	, m_DQBUF_last_errno(0)
{
	m_pImageProcessingThread = QSharedPointer<ImageProcessingThread>(new ImageProcessingThread());

	connect(m_pImageProcessingThread.data(), SIGNAL(OnFrameReady_Signal(const QImage &, const unsigned long long &, const int &)), this, SLOT(OnFrameReadyFromThread(const QImage &, const unsigned long long &, const int &)));
	connect(m_pImageProcessingThread.data(), SIGNAL(OnMessage_Signal(const QString &)), this, SLOT(OnMessageFromThread(const QString &)));
}

FrameObserver::~FrameObserver()
{
    StopStream();

    m_pImageProcessingThread->StopThread();

    // wait until the thread is stopped
    while (isRunning())
	QThread::msleep(10);
}

int FrameObserver::StartStream(bool blockingMode, int fileDescriptor, uint32_t pixelformat, 
			       uint32_t payloadsize, uint32_t width, uint32_t height, uint32_t bytesPerLine,
			       uint32_t enableLogging, int32_t logFrameStart, int32_t logFrameEnd,
			       int32_t dumpFrameStart, int32_t dumpFrameEnd, uint32_t enableRAW10Correction,
				   std::vector<uint8_t> &rData)
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
    
    m_EnableLogging = enableLogging;
    m_FrameCount = 0;
    m_LogFrameStart = logFrameStart;
    m_LogFrameEnd = logFrameEnd;
    m_DumpFrameStart = dumpFrameStart;
    m_DumpFrameEnd = dumpFrameEnd;
    
    m_rCSVData = rData;
	
	m_EnableRAW10Correction = enableRAW10Correction;
    
    if (0 == g_ConversionBuffer1)
        g_ConversionBuffer1 = (uint8_t*)malloc(m_nWidth*m_nHeight*4);
    
    if (0 == g_ConversionBuffer2)
        g_ConversionBuffer2 = (uint8_t*)malloc(m_nWidth*m_nHeight*4);
    
    m_pImageProcessingThread->StartThread();
    
    start();

    {
	AVT::BaseTools::AutoLocalMutex guard(m_UsedBufferMutex);
	m_UserBufferContainerList.resize(0);
    }
    
    return nResult;
}

int FrameObserver::StopStream()
{
    int nResult = 0;
    int count = 300;
    
    m_pImageProcessingThread->StopThread();
    
    m_bStreamRunning = false;
    
    while (!m_bStreamStopped && count-- > 0)
         QThread::msleep(10);

    if (count <= 0)
       nResult = -1;

    if (0 != g_ConversionBuffer1)
        free(g_ConversionBuffer1);
    g_ConversionBuffer1 = 0;
    
    if (0 != g_ConversionBuffer2)
        free(g_ConversionBuffer2);
    g_ConversionBuffer2 = 0;
    
    return nResult;
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
    int result = 0;
			
    result = ReadFrame(buf);
    if (0 == result)
    {
	m_FrameId++;
	m_nReceivedFramesCounter++;
		
	if (m_ShowFrames)
	{
	    uint8_t *buffer = 0;
	    uint32_t length = 0;
		uint32_t logPayloadSize = m_PayloadSize;
		
	    if (0 == GetFrameData(buf, buffer, length))
	    {
		  
		if (m_EnableRAW10Correction &&
		    (m_Pixelformat == V4L2_PIX_FMT_Y10P ||
			 m_Pixelformat == V4L2_PIX_FMT_SBGGR10P ||
			 m_Pixelformat == V4L2_PIX_FMT_SGBRG10P ||
			 m_Pixelformat == V4L2_PIX_FMT_SGRBG10P ||
			 m_Pixelformat == V4L2_PIX_FMT_SRGGB10P))
		{
			length = InternalConvertRAW10inRAW16ToRAW10g(buffer, m_PayloadSize, g_ConversionBuffer2);
			buffer = g_ConversionBuffer2;
			logPayloadSize = length;
		}
		  
		if (length <= m_RealPayloadsize)
		{
		    if (m_FrameCount >= m_LogFrameStart && 
			m_FrameCount <=  m_LogFrameEnd)
		    {
        		Logger::LogDump("Received frame:", (uint8_t*)buffer, logPayloadSize);
		    }
				
		    if (m_FrameCount >= m_DumpFrameStart && 
			m_FrameCount <=  m_DumpFrameEnd)
		    {
			std::stringstream localFileName;
			
			localFileName << "v4l2test_Frame" << m_FrameCount << ".dmp";
		
        		Logger::LogBuffer(localFileName.str(), (uint8_t*)buffer, logPayloadSize);
		    }
		    m_FrameCount++;
		
		    if (m_rCSVData.size() > 0 && m_FrameCount == 1)
		    {
			uint32_t tmpCSVPayloadSize = m_rCSVData.size();
			
			if (m_PayloadSize == tmpCSVPayloadSize)
			{
			    bool notequalflag = false;
			    for (uint32_t i=0; i<m_PayloadSize; i++)
			    {
				if (buffer[i] != m_rCSVData[i])
				{
				    Logger::Log("!!! Buffer unequal to CSV file. !!!");
				    emit OnMessage_Signal("!!! Buffer unequal to CSV file. !!!");
				    notequalflag = true;
				    break;
				}
			    }
			    if (!notequalflag)
			    {
				Logger::Log("*** Buffer equal to CSV file. ***");
				emit OnMessage_Signal("*** Buffer equal to CSV file. ***");
			    }
			}
			else
			{
			    Logger::LogEx("Buffer size=%d unequal to CSV file data size=%d.", m_PayloadSize, tmpCSVPayloadSize);
			    emit OnMessage_Signal(QString("Buffer size=%1 unequal to CSV file data size=%2.").arg(m_PayloadSize).arg(tmpCSVPayloadSize));
			}
		    }
				
		    if (m_bRecording)
		    {
    			if (m_FrameRecordQueue.GetSize() < MAX_RECORD_FRAME_QUEUE_SIZE)
			{
			    QImage convertedImage;
			
			    if (AVT::Tools::ImageTransf::ConvertFrame(buffer, length, 
								  m_nWidth, m_nHeight, m_Pixelformat, 
								  m_PayloadSize, m_BytesPerLine, convertedImage) == 0)
			    {
				m_FrameRecordQueue.Enqueue(convertedImage, m_FrameId);
				emit OnRecordFrame_Signal(m_FrameId, m_FrameRecordQueue.GetSize());
			    }
			    else
				emit OnError_Signal("Frame buffer not converted. Possible missing conversion.");
			}
			else
			{
			    if (m_FrameRecordQueue.GetSize() == MAX_RECORD_FRAME_QUEUE_SIZE)
				emit OnMessage_Signal(QString("Following frames are not saved, more than %1 would freeze the system.").arg(MAX_RECORD_FRAME_QUEUE_SIZE));
			}
		    }
			
		    /*
		     * Test data for Mono12P and Mono10P from csv file
		     * Therefore please comment csv comparison and recording out */
		    /*
		    buffer = &m_rCSVData[0];
		    length = m_rCSVData.size();
		    m_nWidth = 1296;
		    m_nHeight = 968;
		    m_Pixelformat = V4L2_PIX_FMT_Y12P; //V4L2_PIX_FMT_Y10P; //V4L2_PIX_FMT_Y12P;
		    m_PayloadSize = 1881792; //1568160; //1881792;
		    m_BytesPerLine = 1944; //1620; //1944;
		    */
		    if (m_pImageProcessingThread->QueueFrame(buf.index, buffer, length, 
							m_nWidth, m_nHeight, m_Pixelformat, 
							m_PayloadSize, m_BytesPerLine, m_FrameId))
		    {
			// when frame was not queued to image queue because queue is full 
			// frame should be queued to v4l2 queue again
			//emit OnFrameID_Signal(m_FrameId);
			QueueSingleUserBuffer(buf.index);
		    }
		}
		else
		{
		    m_nDroppedFramesCounter++;
			    
		    emit OnError_Signal(QString("Received data length is higher than announced payload size. lenght=%1, payloadsize=%2").arg(length).arg(m_PayloadSize));

		    emit OnFrameID_Signal(m_FrameId);
		    QueueSingleUserBuffer(buf.index);
		}
	    }
	    else
	    {
		m_nDroppedFramesCounter++;
			    
		emit OnError_Signal("Missing buffer data.");

		emit OnFrameID_Signal(m_FrameId);
		QueueSingleUserBuffer(buf.index);
	    }
	}
	else
	{
	    m_nDroppedFramesCounter++;
				    
	    //emit OnFrameID_Signal(m_FrameId);
	    QueueSingleUserBuffer(buf.index);
	}
    }
    else
    {
	if (!m_BlockingMode && errno != EAGAIN)
	{
		static int i=0;
		i++;
		if (i%10000 == 0 || m_DQBUF_last_errno != errno)
		{
	    		m_DQBUF_last_errno = errno;
	    		emit OnError_Signal(QString("DQBUF error %1 times, error=%2").arg(i).arg(errno));
		}
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

	    if (m_BlockingMode)
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
	    // non-blocking mode
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

void FrameObserver::OnMessageFromThread(const QString &msg)
{
    emit OnMessage_Signal(msg);
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


MyFrameQueue& FrameObserver::GetRecordQueue()
{
	return m_FrameRecordQueue.GetQueue();
}

/*********************************************************************************************************/
// Frame buffer handling
/*********************************************************************************************************/

int FrameObserver::CreateAllUserBuffer(uint32_t bufferCount, uint32_t bufferSize)
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

int FrameObserver::DeleteAllUserBuffer()
{
    int result = 0;

    return result;
}

// The event handler to return the frame
/*
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
*/

void FrameObserver::SwitchFrameTransfer2GUI(bool showFrames)
{
    m_ShowFrames = showFrames;
}

}}} // namespace AVT::Tools::Examples

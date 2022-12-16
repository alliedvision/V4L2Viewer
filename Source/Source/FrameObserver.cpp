/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2021 Allied Vision Technologies GmbH

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.  */


#include "DeviationCalculator.h"
#include "FrameObserver.h"
#include "ImageTransform.h"
#include "Logger.h"

#include <QPixmap>
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define CLIP(color) (unsigned char)(((color) > 0xFF) ? 0xff : (((color) < 0) ? 0 : (color)))

#define V4L2_PIX_FMT_Y10P     v4l2_fourcc('Y', '1', '0', 'P')

uint8_t *g_ConversionBuffer1 = 0;
uint8_t *g_ConversionBuffer2 = 0;

uint32_t InternalConvertRAW10inRAW16ToRAW10g(const void *sourceBuffer, uint32_t length, const void *destBuffer)
{
    uint8_t *destData = (unsigned char *)destBuffer;
    uint8_t *srcData = (unsigned char *)sourceBuffer;
    uint32_t srcCount = 0;
    uint32_t destCount = 0;

    while (srcCount < length)
    {
        unsigned char lsbits = 0;

        for (int iii = 0; iii < 4; iii++)
        {
            *destData++ = *(srcData + 1); // bits [9:2]
            destCount++;

            lsbits |= (*srcData >> 6) << (iii * 2); // least significant bits [1:0]

            srcData += 2; // move to next 16 bits

            srcCount += 2;
        }

        *destData++ = lsbits; // every 5th byte contains the lsbs from the last 4 pixels
        destCount++;
    }

    return destCount;
}

FrameObserver::FrameObserver(bool showFrames)
    : m_nFileDescriptor(0)
    , m_BufferType(V4L2_BUF_TYPE_VIDEO_CAPTURE)
    , m_PixelFormat(0)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_PayloadSize(0)
    , m_RealPayloadSize(0)
    , m_BytesPerLine(0)
    , m_FrameId(0)
    , m_DQBUF_last_errno(0)
    , m_MessageSendFlag(false)
    , m_BlockingMode(false)
    , m_IsStreamRunning(false)
    , m_bStreamStopped(true)
    , m_EnableLogging(0)
    , m_ShowFrames(showFrames)
{
    m_pImageProcessingThread = QSharedPointer<ImageProcessingThread>(new ImageProcessingThread());

    connect(m_pImageProcessingThread.data(), SIGNAL(OnFrameReady_Signal(const QImage &, const unsigned long long &, const int &)), this, SLOT(OnFrameReadyFromThread(const QImage &, const unsigned long long &, const int &)));
}

FrameObserver::~FrameObserver()
{
    StopStream();

    m_pImageProcessingThread->StopThread();

    // wait until the thread is stopped
    while (isRunning())
        QThread::msleep(10);
}

int FrameObserver::StartStream(bool blockingMode, int fileDescriptor, uint32_t pixelFormat,
                               uint32_t payloadSize, uint32_t width, uint32_t height, uint32_t bytesPerLine,
                               uint32_t enableLogging)
{
    int nResult = 0;

    m_BlockingMode = blockingMode;
    m_nFileDescriptor = fileDescriptor;
    m_nWidth = width;
    m_nHeight = height;
    m_FrameId = 0;
    m_ReceivedFPS.clear();
    m_RenderedFPS.clear();
    m_PayloadSize = payloadSize;
    m_PixelFormat = pixelFormat;
    m_BytesPerLine = bytesPerLine;
    m_MessageSendFlag = false;

    m_bStreamStopped = false;
    m_IsStreamRunning = true;

    m_EnableLogging = enableLogging;

    if (0 == g_ConversionBuffer1)
        g_ConversionBuffer1 = (uint8_t*)malloc(m_nWidth * m_nHeight * 4);

    if (0 == g_ConversionBuffer2)
        g_ConversionBuffer2 = (uint8_t*)malloc(m_nWidth * m_nHeight * 4);

    m_pImageProcessingThread->StartThread();

    start();

    return nResult;
}

int FrameObserver::StopStream()
{
    int nResult = 0;
    int count = 300;

    m_pImageProcessingThread->StopThread();

    m_IsStreamRunning = false;

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
        m_ReceivedFPS.trigger();

        if (m_ShowFrames)
        {
            uint8_t *buffer = 0;
            uint32_t length = 0;

            if (0 == GetFrameData(buf, buffer, length))
            {

                if (m_PixelFormat == V4L2_PIX_FMT_Y10P ||
                    m_PixelFormat == V4L2_PIX_FMT_SBGGR10P ||
                    m_PixelFormat == V4L2_PIX_FMT_SGBRG10P ||
                    m_PixelFormat == V4L2_PIX_FMT_SGRBG10P ||
                    m_PixelFormat == V4L2_PIX_FMT_SRGGB10P)
                {
                    length = InternalConvertRAW10inRAW16ToRAW10g(buffer, m_PayloadSize, g_ConversionBuffer2);
                    buffer = g_ConversionBuffer2;
                }

                if (length <= m_RealPayloadSize)
                {
                    if (m_pImageProcessingThread->QueueFrame(buf.index, buffer, length,
                        m_nWidth, m_nHeight, m_PixelFormat,
                        m_PayloadSize, m_BytesPerLine, m_FrameId))
                    {
                        // when frame was not queued to image queue because queue is full
                        // frame should be queued to v4l2 queue again
                        QueueSingleUserBuffer(buf.index);
                    }
                }
                else
                {
                    emit OnFrameID_Signal(m_FrameId);
                    QueueSingleUserBuffer(buf.index);
                }
            }
            else
            {
                emit OnFrameID_Signal(m_FrameId);
                QueueSingleUserBuffer(buf.index);
            }
        }
        else
        {
            emit OnFrameID_Signal(m_FrameId);
            QueueSingleUserBuffer(buf.index);
        }
    }
    else
    {
        if (!m_BlockingMode && errno != EAGAIN)
        {
            static int i = 0;
            i++;
            if (i % 10000 == 0 || static_cast<int32_t>(m_DQBUF_last_errno) != errno)
            {
                m_DQBUF_last_errno = errno;
            }
        }
    }
}

// Do the work within this thread
void FrameObserver::run()
{
    m_IsStreamRunning = true;

    while (m_IsStreamRunning)
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
            }
            else if (0 == result)
            {
                // Timeout
                QThread::msleep(0);
                continue;
            }
            else
            {
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
}

// Get the number of frames
// This function will clear the counter of received frames
double FrameObserver::GetReceivedFPS()
{
    return m_ReceivedFPS.getFPS();
}

double FrameObserver::GetRenderedFPS()
{
    return m_RenderedFPS.getFPS();
}

void FrameObserver::OnFrameReadyFromThread(const QImage &image, const unsigned long long &frameId, const int &bufIndex)
{
    m_RenderedFPS.trigger();
    emit OnFrameReady_Signal(image, frameId);

    QueueSingleUserBuffer(bufIndex);
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


void FrameObserver::SwitchFrameTransfer2GUI(bool showFrames)
{
    m_ShowFrames = showFrames;
}


void FrameObserver::setFileDescriptor(int fd)
{
    m_nFileDescriptor = fd;
}


void FrameObserver::setBufferType(v4l2_buf_type bufferType)
{
    m_BufferType = bufferType;
}

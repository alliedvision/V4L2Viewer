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


#include "FrameObserver.h"
#include "Logger.h"

#include <QPixmap>
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>


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
}

FrameObserver::~FrameObserver()
{
    StopStream();
    wait();
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
    m_PayloadSize = payloadSize;
    m_PixelFormat = pixelFormat;
    m_BytesPerLine = bytesPerLine;
    m_MessageSendFlag = false;

    m_bStreamStopped = false;
    m_IsStreamRunning = true;

    m_EnableLogging = enableLogging;

    start();

    return nResult;
}

int FrameObserver::StopStream()
{
    int nResult = 0;
    int count = 300;

    m_IsStreamRunning = false;

    while (!m_bStreamStopped && count-- > 0) {
        QThread::msleep(10);
    }

    if (count <= 0) {
        nResult = -1;
    }

    return nResult;
}

int FrameObserver::AddRawDataProcessor(DataProcessorFunc processor)
{
    int index = m_rawDataProcessors.size();
    m_rawDataProcessors.push_back(processor);

    return index;
}


uint64_t constexpr allOnes(int count) {
    assert(count < sizeof(uint64_t) * 8);
    return (1ULL << uint64_t(count)) - 1ULL;
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

          uint8_t *buffer = 0;
          uint32_t length = 0;

          if (0 == GetFrameData(buf, buffer, length))
          {
              auto const procCount = m_rawDataProcessors.size();
              if(procCount > 0) {
                  m_UserBufferContainerList[buf.index]->processMap = allOnes(procCount);
                  int i = 0;
                  for (auto const & cb : m_rawDataProcessors) {
                      cb(BufferWrapper { buf, buffer, length, m_nWidth, m_nHeight,
                                         m_PixelFormat, m_PayloadSize, m_BytesPerLine, m_FrameId },
                          [i, idx = buf.index, this] {
                              auto& map = m_UserBufferContainerList[idx]->processMap;
                              map &= ~(1ULL << i);
                              if(map == 0) {
                                QueueSingleUserBuffer(idx);
                              }
                          });
                      ++i;
                  }
              } else {
                  QueueSingleUserBuffer(buf.index);
              }
            }
            else
            {
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

            if (result == -1)
            {
                // Error
                continue;
            }
            else if (result == 0)
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


/*********************************************************************************************************/
// Frame buffer handling
/*********************************************************************************************************/


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

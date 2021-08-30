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


#include "ImageProcessingThread.h"
#include "ImageTransform.h"

#include <linux/videodev2.h>

ImageProcessingThread::ImageProcessingThread()
    : m_bAbort(false)
{
}

ImageProcessingThread::~ImageProcessingThread(void)
{
}

int ImageProcessingThread::QueueFrame(uint32_t &bufferIndex, uint8_t *&buffer, uint32_t &length,
                                      uint32_t &width, uint32_t &height, uint32_t &pixelFormat,
                                      uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID)
{
    int result = -1;

    // make sure Viewer is never working in the past
    // if viewer is too slow we drop the frames.
    if (m_FrameQueue.GetSize() < MAX_QUEUE_SIZE)
    {
        m_FrameQueue.Enqueue(bufferIndex, buffer, length, width, height,
                             pixelFormat, payloadSize, bytesPerLine, frameID);
        result = 0;
    }

    return result;
}

// Set the data for the thread to work with
int ImageProcessingThread::QueueFrame(QImage &image, uint64_t &frameID)
{
    int result = -1;

    // make sure Viewer is never working in the past
    // if viewer is too slow we drop the frames.
    if (m_FrameQueue.GetSize() < MAX_QUEUE_SIZE)
    {
        m_FrameQueue.Enqueue(image, frameID);
        result = 0;
    }

    return result;
}

// Set the data for the thread to work with
int ImageProcessingThread::QueueFrame(QSharedPointer<MyFrame> pFrame)
{
    int result = -1;

    // make sure Viewer is never working in the past
    // if viewer is too slow we drop the frames.
    if (m_FrameQueue.GetSize() < MAX_QUEUE_SIZE)
    {
        m_FrameQueue.Enqueue(pFrame);
        result = 0;
    }

    return result;
}

// stop the internal processing thread and wait until the thread is really stopped
void ImageProcessingThread::StartThread()
{
    m_bAbort = false;

    start();
}

// stop the internal processing thread and wait until the thread is really stopped
void ImageProcessingThread::StopThread()
{
    m_bAbort = true;

    // wait until the thread is stopped
    while (isRunning())
        QThread::msleep(10);

    m_FrameQueue.Clear();
}

// Do the work within this thread
void ImageProcessingThread::run()
{
    int result = 0;

    while (!m_bAbort)
    {
        if(0 < m_FrameQueue.GetSize())
        {
            QSharedPointer<MyFrame> pFrame = m_FrameQueue.Dequeue();
            uint64_t frameID = pFrame->GetFrameId();
            const uint8_t* pBuffer = pFrame->GetBuffer();
            uint32_t length = pFrame->GetBufferlength();
            uint32_t width = pFrame->GetWidth();
            uint32_t height = pFrame->GetHeight();
            uint32_t pixelFormat = pFrame->GetPixelFormat();
            uint32_t payloadSize = pFrame->GetPayloadSize();
            uint32_t bytesPerLine = pFrame->GetBytesPerLine();
            uint32_t bufferIndex = pFrame->GetBufferIndex();
            QImage convertedImage;
            result = ImageTransform::ConvertFrame(pBuffer, length,
                                                  width, height, pixelFormat,
                                                  payloadSize, bytesPerLine, convertedImage);

            if (result == 0)
                emit OnFrameReady_Signal(convertedImage, frameID, bufferIndex);
        }

        QThread::msleep(1);
    }
}


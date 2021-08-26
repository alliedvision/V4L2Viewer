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


#ifndef IMAGEPORCESSINGTHREAD_H
#define IMAGEPORCESSINGTHREAD_H

#include <MyFrame.h>
#include <MyFrameQueue.h>

#include <QImage>
#include <QObject>
#include <QSharedPointer>
#include <QThread>

class ImageProcessingThread : public QThread
{
    Q_OBJECT

public:
    ImageProcessingThread();
    ~ImageProcessingThread(void);

    // This function queues the frame for the thread to work with
    //
    // Parameters:
    // [in] (uint32_t &) bufferIndex - index of the buffer
    // [in] (uint8_t *&) buffer - given buffer
    // [in] (uint32_t &) length - length of the buffer
    // [in] (uint32_t &) width - width of the frame
    // [in] (uint32_t &) height - height of the frame
    // [in] (uint32_t &) pixelFormat
    // [in] (uint32_t &) payloadSize
    // [in] (uint32_t &) bytesPerLine
    // [in] (uint64_t &) frameID
    //
    // Returns:
    // (int) - result of queuing
    int QueueFrame(uint32_t &bufferIndex, uint8_t *&buffer, uint32_t &length,
                   uint32_t &width, uint32_t &height, uint32_t &pixelFormat,
                   uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID);

    // This function queues the frame for the thread to work with
    //
    // Parameters:
    // [in] (QImage &) image
    // [in] (uint64_t &) frameID
    //
    // Returns:
    // (int) - result of queuing
    int QueueFrame(QImage &image, uint64_t &frameID);

    // This function queues the frame for the thread to work with
    //
    // Parameters:
    // [in] (QSharedPointer<MyFrame>) pFrame
    //
    // Returns:
    // (int) - result of queuing
    int QueueFrame(QSharedPointer<MyFrame> pFrame);

    // This function starts thread
    void StartThread();

    // This function stops the internal processing thread and wait until the thread is really stopped
    void StopThread();

protected:
    // This function does the work within this thread
    virtual void run();

private:
    const static int MAX_QUEUE_SIZE = 1;

    // Frame queue
    MyFrameQueue m_FrameQueue;

    // Variable to abort the running thread
    bool m_bAbort;

signals:
    // Event will be called when an image is processed by the thread
    void OnFrameReady_Signal(const QImage &image, const unsigned long long &frameId, const int &bufIndex);
};

#endif // IMAGEPORCESSINGTHREAD_H


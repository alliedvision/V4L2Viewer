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


#ifndef MYFRAMEQUEUE_H
#define MYFRAMEQUEUE_H

#include "MyFrame.h"

#include <QMutex>
#include <QQueue>
#include <QSharedPointer>

class MyFrameQueue
{
public:
    // Copy the of the given frame
    MyFrameQueue(void);
    ~MyFrameQueue(void);

    // This function returns the size of the queue
    unsigned int GetSize();

    // This function clears queue
    void Clear();

    // This function adds a new frame
    //
    // Parameters:
    // [in] (uint32_t &) bufferIndex
    // [in] (uint8_t *&) buffer
    // [in] (uint32_t &) length
    // [in] (uint32_t &) width
    // [in] (uin32_t &) height
    // [in] (uint32_t &) pixelFormat
    // [in] (uint32_t &) payloadSize
    // [in] (uint32_t &) bytesPerLine
    // [in] (uint64_t &) frameID
    void Enqueue(uint32_t &bufferIndex, uint8_t *&buffer, uint32_t &length,
         uint32_t &width, uint32_t &height, uint32_t &pixelFormat,
         uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID);

    // This function adds a new frame to the queue
    //
    // Parameters:
    // [in] (QImage &) image
    // [in] (uint64_t) frameID
    void Enqueue(QImage &image, uint64_t frameID);

    // This function adds a new frame to the queue
    //
    // Parameters:
    // [in] (QSharedPointer<MyFrame>) pFrame
    void Enqueue(QSharedPointer<MyFrame> pFrame);

    // This function return and remove frame out of the queue
    //
    // Returns:
    // QSharedPointer<MyFrame>
    QSharedPointer<MyFrame> Dequeue();

    // This function returns the frame at the previous index
    //
    // Returns:
    // QSharedPointer<MyFrame>
    QSharedPointer<MyFrame> GetPrevious();

    // This function returns the frame at the next index
    //
    // Returns:
    // QSharedPointer<MyFrame>
    QSharedPointer<MyFrame> GetNext();

private:
    QQueue< QSharedPointer<MyFrame> > m_FrameQueue;
    QMutex m_FrameQueueMutex;
    int m_nQueueIndex;
};

#endif // MYFRAMEQUEUE_H

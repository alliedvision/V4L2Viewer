/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        MyFrameQueue.h

  Description: This class is responsible for queuing and dequeuing arrived
               frames.

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

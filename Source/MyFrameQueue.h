/*=============================================================================
  Copyright (C) 2015 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        MyFrameQueue.h

  Description:

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

#include <QMutex>
#include <QQueue>
#include <QSharedPointer>

#include <MyFrame.h>

class MyFrameQueue
{
public:
    // Copy the of the given frame
    MyFrameQueue(void);
    ~MyFrameQueue(void);

    // Get the size of the queue
    unsigned int GetSize();

    // Clear queue
    void Clear();

    // Add a new frame
    void Enqueue(uint32_t &bufferIndex, uint8_t *&buffer, uint32_t &length, 
		 uint32_t &width, uint32_t &height, uint32_t &pixelformat,
		 uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID);
	
    // Add a new frame
    void Enqueue(QImage &image, uint64_t frameID);

    // Add a new frame
    void Enqueue(QSharedPointer<MyFrame> pFrame);

    // Get the frame out of the queue
    QSharedPointer<MyFrame> Dequeue();

    // Get the frame at the previous index
    QSharedPointer<MyFrame> GetPrevious();

    // Get the frame at the next index
    QSharedPointer<MyFrame> GetNext();

    MyFrameQueue& GetQueue();

private:
    QQueue< QSharedPointer<MyFrame> > m_FrameQueue;
    QMutex m_FrameQueueMutex;
    int m_nQueueIndex;
};

#endif // MYFRAMEQUEUE_H

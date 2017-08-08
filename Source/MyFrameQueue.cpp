/*=============================================================================
  Copyright (C) 2015 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        MyFrameQueue.cpp

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

#include "MyFrameQueue.h"

MyFrameQueue::MyFrameQueue()
    : m_nQueueIndex(0)
{
}

MyFrameQueue::~MyFrameQueue()
{
}

// Get the size of the queue
unsigned int MyFrameQueue::GetSize()
{
    unsigned int size = 0;

    m_FrameQueueMutex.lock();

    size = m_FrameQueue.size();

    m_FrameQueueMutex.unlock();

    return size;
}

// Get the size of the queue
void MyFrameQueue::Clear()
{
    m_FrameQueueMutex.lock();

    m_FrameQueue.clear();
    m_nQueueIndex = 0;

    m_FrameQueueMutex.unlock();
}

void MyFrameQueue::Enqueue(uint32_t &bufferIndex, uint8_t *&buffer, uint32_t &length, 
			   uint32_t &width, uint32_t &height, uint32_t &pixelformat,
			   uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID)
{
    m_FrameQueueMutex.lock();

    QSharedPointer<MyFrame> newFrame = QSharedPointer<MyFrame>(new MyFrame(bufferIndex, 
									   buffer, 
									   length, 
									   width, 
									   height, 
									   pixelformat, 
									   payloadSize, 
									   bytesPerLine, 
									   frameID));
		
    m_FrameQueue.enqueue(newFrame);
	
    m_FrameQueueMutex.unlock();
}

// Add a new frame
void MyFrameQueue::Enqueue( QImage &image, 
                            uint64_t frameID)
{
    m_FrameQueueMutex.lock();

    QSharedPointer<MyFrame> newFrame = QSharedPointer<MyFrame>(new MyFrame(image, frameID));
		
    m_FrameQueue.enqueue(newFrame);
	
    m_FrameQueueMutex.unlock();
}

// Add a new frame
void MyFrameQueue::Enqueue(QSharedPointer<MyFrame> pFrame)
{
    m_FrameQueueMutex.lock();

    m_FrameQueue.enqueue(pFrame);
	
    m_FrameQueueMutex.unlock();
}

// Get the frame out of the queue
QSharedPointer<MyFrame> MyFrameQueue::Dequeue()
{
    QSharedPointer<MyFrame> queuedFrame;

    m_FrameQueueMutex.lock();

    if (m_FrameQueue.size() > 0)
	queuedFrame = m_FrameQueue.dequeue();

    m_FrameQueueMutex.unlock();

    return queuedFrame;
}

// Get the frame out of the queue
QSharedPointer<MyFrame> MyFrameQueue::GetPrevious()
{
    QSharedPointer<MyFrame> queuedFrame;

    m_FrameQueueMutex.lock();

    if (m_FrameQueue.size() > 0)
    {
        m_nQueueIndex--;
        if (m_nQueueIndex < 0)
            m_nQueueIndex = 0;

        queuedFrame = m_FrameQueue.at(m_nQueueIndex);
    }

    m_FrameQueueMutex.unlock();

    return queuedFrame;
}

// Get the frame out of the queue
QSharedPointer<MyFrame> MyFrameQueue::GetNext()
{
    QSharedPointer<MyFrame> queuedFrame;

    m_FrameQueueMutex.lock();

    if (m_FrameQueue.size() > 0)
    {
        m_nQueueIndex++;
        if (m_nQueueIndex >= m_FrameQueue.size())
            m_nQueueIndex = m_FrameQueue.size()-1;

        queuedFrame = m_FrameQueue.at(m_nQueueIndex);
    }

    m_FrameQueueMutex.unlock();

    return queuedFrame;
}

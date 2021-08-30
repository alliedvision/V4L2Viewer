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
                           uint32_t &width, uint32_t &height, uint32_t &pixelFormat,
                           uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID)
{
    m_FrameQueueMutex.lock();

    QSharedPointer<MyFrame> newFrame = QSharedPointer<MyFrame>(new MyFrame(bufferIndex,
                                       buffer,
                                       length,
                                       width,
                                       height,
                                       pixelFormat,
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

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


#include "FrameObserverMMAP.h"
#include "LocalMutexLockGuard.h"
#include "Logger.h"
#include "MemoryHelper.h"

#include <QPixmap>

#include <errno.h>
#include <fcntl.h>
#include <IOHelper.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <QDebug>

#include <sstream>

FrameObserverMMAP::FrameObserverMMAP(bool showFrames)
    : FrameObserver(showFrames)
{
}

FrameObserverMMAP::~FrameObserverMMAP()
{
}

int FrameObserverMMAP::ReadFrame(v4l2_buffer &buf)
{
    int result = -1;

    memset(&buf, 0, sizeof(buf));

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (m_IsStreamRunning)
        result = iohelper::xioctl(m_nFileDescriptor, VIDIOC_DQBUF, &buf);

    return result;
}

int FrameObserverMMAP::GetFrameData(v4l2_buffer &buf, uint8_t *&buffer, uint32_t &length)
{
    int result = -1;

    if (m_IsStreamRunning)
    {
        base::LocalMutexLockGuard guard(m_UsedBufferMutex);

        if (buf.index < m_UserBufferContainerList.size())
        {
            length = m_UserBufferContainerList[buf.index]->nBufferlength;
            buffer = m_UserBufferContainerList[buf.index]->pBuffer;
        }
        else
        {
            length = 0;
            buffer = 0;
        }

        if (0 != buffer && 0 != length)
        {
            result = 0;
        }
    }

    return result;
}

/*********************************************************************************************************/
// Frame buffer handling
/*********************************************************************************************************/

int FrameObserverMMAP::CreateAllUserBuffer(uint32_t bufferCount, uint32_t bufferSize)
{
    int result = -1;

    if (bufferCount <= MAX_VIEWER_USER_BUFFER_COUNT)
    {
        v4l2_requestbuffers req;

        // creates user defined buffer
        CLEAR(req);

        req.count  = bufferCount;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        // requests 4 video capture buffer. Driver is going to configure all parameter and doesn't allocate them.
        if (-1 == iohelper::xioctl(m_nFileDescriptor, VIDIOC_REQBUFS, &req))
        {
            if (EINVAL == errno)
            {
                Logger::LogEx("FrameObserverMMAP::CreateUserBuffer VIDIOC_REQBUFS does not support user pointer i/o");
            }
            else
            {
                Logger::LogEx("FrameObserverMMAP::CreateUserBuffer VIDIOC_REQBUFS error");
            }
        }
        else
        {
            base::LocalMutexLockGuard guard(m_UsedBufferMutex);

            Logger::LogEx("FrameObserverMMAP::CreateUserBuffer VIDIOC_REQBUFS OK");

            // create local buffer container
            m_UserBufferContainerList.resize(bufferCount);

            if (m_UserBufferContainerList.size() != bufferCount)
            {
                m_UserBufferContainerList.resize(0);
                Logger::LogEx("FrameObserverMMAP::CreateUserBuffer buffer container error");
                return -1;
            }

            for (unsigned int x = 0; x < bufferCount; ++x)
            {
                v4l2_buffer buf;
                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = x;

                if (-1 == iohelper::xioctl(m_nFileDescriptor, VIDIOC_QUERYBUF, &buf))
                {
                    Logger::LogEx("FrameObserverMMAP::CreateUserBuffer VIDIOC_QUERYBUF error");
                    return -1;
                }

                Logger::LogEx("FrameObserverMMAP::CreateUserBuffer VIDIOC_QUERYBUF MMAP OK length=%d", buf.length);

                UserBuffer* pTmpBuffer = new UserBuffer;
                pTmpBuffer->nBufferlength = buf.length;
                m_RealPayloadSize = pTmpBuffer->nBufferlength;
                pTmpBuffer->pBuffer = (uint8_t*)mmap(NULL,
                                    buf.length,
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED,
                                    m_nFileDescriptor,
                                    buf.m.offset);

                if (MAP_FAILED == pTmpBuffer->pBuffer)
                {
                    delete pTmpBuffer;
                    m_UserBufferContainerList.resize(0);
                    return -1;
                }
                else
                    m_UserBufferContainerList[x] = pTmpBuffer;
            }

            result = 0;
        }
    }

    return result;
}

int FrameObserverMMAP::QueueAllUserBuffer()
{
    int result = -1;
    base::LocalMutexLockGuard guard(m_UsedBufferMutex);

    // queue the buffer
    for (uint32_t i=0; i<m_UserBufferContainerList.size(); i++)
    {
        v4l2_buffer buf;

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.index = i;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == iohelper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
        {
            Logger::LogEx("FrameObserverMMAP::QueueUserBuffer VIDIOC_QBUF queue #%d buffer=%p failed", i, m_UserBufferContainerList[i]->pBuffer);
            return result;
        }
        else
        {
            Logger::LogEx("FrameObserverMMAP::QueueUserBuffer VIDIOC_QBUF queue #%d buffer=%p OK", i, m_UserBufferContainerList[i]->pBuffer);
            result = 0;
        }
    }

    return result;
}

int FrameObserverMMAP::QueueSingleUserBuffer(const int index)
{
    int result = 0;
    v4l2_buffer buf;
    base::LocalMutexLockGuard guard(m_UsedBufferMutex);

    if (index < static_cast<int>(m_UserBufferContainerList.size()))
    {
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.index = index;
        buf.memory = V4L2_MEMORY_MMAP;

        if (m_IsStreamRunning)
        {
            if (-1 == iohelper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
            {
                Logger::LogEx("FrameObserverMMAP::QueueSingleUserBuffer VIDIOC_QBUF queue #%d buffer=%p failed", index, m_UserBufferContainerList[index]->pBuffer);
            }
        }
    }

    return result;
}

int FrameObserverMMAP::DeleteAllUserBuffer()
{
    int result = 0;

    base::LocalMutexLockGuard guard(m_UsedBufferMutex);

    // delete all user buffer
    for (unsigned int x = 0; x < m_UserBufferContainerList.size(); x++)
    {
        munmap(m_UserBufferContainerList[x]->pBuffer, m_UserBufferContainerList[x]->nBufferlength);

        if (0 != m_UserBufferContainerList[x])
        {
            delete m_UserBufferContainerList[x];
        }
    }

    m_UserBufferContainerList.resize(0);


    // free all internal buffers
    v4l2_requestbuffers req;
    // creates user defined buffer
    CLEAR(req);
    req.count  = 0;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    // requests 4 video capture buffer. Driver is going to configure all parameter and doesn't allocate them.
    result = iohelper::xioctl(m_nFileDescriptor, VIDIOC_REQBUFS, &req);

    return result;
}


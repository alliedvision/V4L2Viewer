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
#include "V4L2Helper.h"

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

    buf.type = m_BufferType;
    buf.memory = V4L2_MEMORY_MMAP;

    v4l2_plane plane;
    if(m_BufferType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
    {
        buf.m.planes = &plane;
        buf.length = 1;
    }

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
        req.type   = m_BufferType;
        req.memory = V4L2_MEMORY_MMAP;

        // requests 4 video capture buffer. Driver is going to configure all parameter and doesn't allocate them.
        if (-1 == iohelper::xioctl(m_nFileDescriptor, VIDIOC_REQBUFS, &req))
        {
            if (EINVAL == errno)
            {
                LOG_EX("FrameObserverMMAP::CreateAllUserBuffer VIDIOC_REQBUFS does not support memory map i/o");
            }
            else
            {
                LOG_EX("FrameObserverMMAP::CreateAllUserBuffer VIDIOC_REQBUFS errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
        }
        else
        {
            base::LocalMutexLockGuard guard(m_UsedBufferMutex);

            LOG_EX("FrameObserverMMAP::CreateAllUserBuffer VIDIOC_REQBUFS OK");

            // create local buffer container
            m_UserBufferContainerList.resize(bufferCount);

            if (m_UserBufferContainerList.size() != bufferCount)
            {
                m_UserBufferContainerList.resize(0);
                LOG_EX("FrameObserverMMAP::CreateAllUserBuffer buffer container error");
                return -1;
            }

            for (unsigned int x = 0; x < bufferCount; ++x)
            {
                v4l2_buffer buf;
                CLEAR(buf);
                buf.type = m_BufferType;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = x;

                v4l2_plane plane;
                if(m_BufferType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
                {
                    buf.m.planes = &plane;
                    buf.length = 1;
                    LOG_EX("FrameObserverMMAP::CreateAllUserBuffer plane count=%d", buf.length);
                }

                if (-1 == iohelper::xioctl(m_nFileDescriptor, VIDIOC_QUERYBUF, &buf))
                {
                    LOG_EX("FrameObserverMMAP::CreateAllUserBuffer VIDIOC_QUERYBUF errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
                    return -1;
                }

                LOG_EX("FrameObserverMMAP::CreateAllUserBuffer VIDIOC_QUERYBUF MMAP OK length=%d", buf.length);

                UserBuffer* pTmpBuffer = new UserBuffer;
                pTmpBuffer->nBufferlength = (m_BufferType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ? buf.m.planes[0].length : buf.length);
                m_RealPayloadSize = pTmpBuffer->nBufferlength;
                pTmpBuffer->pBuffer = (uint8_t*)mmap(NULL,
                        pTmpBuffer->nBufferlength,
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED,
                                    m_nFileDescriptor,
                                    m_BufferType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ? buf.m.planes[0].m.mem_offset : buf.m.offset);

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
        buf.type = m_BufferType;
        buf.index = i;
        buf.memory = V4L2_MEMORY_MMAP;

        v4l2_plane plane;
        if(m_BufferType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
        {
            buf.m.planes = &plane;
            buf.length = 1;
        }

        if (-1 == iohelper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
        {
            LOG_EX("FrameObserverMMAP::QueueUserBuffer VIDIOC_QBUF queue #%d buffer=%p failed, errno=%d=%s", i, m_UserBufferContainerList[i]->pBuffer, errno, v4l2helper::ConvertErrno2String(errno).c_str());
            return result;
        }
        else
        {
            LOG_EX("FrameObserverMMAP::QueueUserBuffer VIDIOC_QBUF queue #%d buffer=%p OK", i, m_UserBufferContainerList[i]->pBuffer);
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
        buf.type = m_BufferType;
        buf.index = index;
        buf.memory = V4L2_MEMORY_MMAP;

        v4l2_plane plane;
        if(m_BufferType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
        {
            buf.m.planes = &plane;
            buf.length = 1;
        }

        if (m_IsStreamRunning)
        {
            if (-1 == iohelper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
            {
                LOG_EX("FrameObserverMMAP::QueueSingleUserBuffer VIDIOC_QBUF queue #%d buffer=%p failed, errno=%d=%s", index, m_UserBufferContainerList[index]->pBuffer, errno, v4l2helper::ConvertErrno2String(errno).c_str());
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
    req.type   = m_BufferType;
    req.memory = V4L2_MEMORY_MMAP;

    // requests 4 video capture buffer. Driver is going to configure all parameter and doesn't allocate them.
    result = iohelper::xioctl(m_nFileDescriptor, VIDIOC_REQBUFS, &req);

    return result;
}


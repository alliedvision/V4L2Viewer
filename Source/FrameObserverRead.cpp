/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameObserverRead.cpp

  Description: The frame observer that is used for notifications
               regarding the arrival of a newly acquired frame.

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

#include "FrameObserverRead.h"
#include "LocalMutexLockGuard.h"
#include "Logger.h"

#include <QPixmap>

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <sstream>

FrameObserverRead::FrameObserverRead(bool showFrames)
    : FrameObserver(showFrames)
    , m_nFrameBufferIndex(0)
{
}

FrameObserverRead::~FrameObserverRead()
{
}

int FrameObserverRead::ReadFrame(v4l2_buffer &buf)
{
    int result = -1;

    memset(&buf, 0, sizeof(buf));

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_USERPTR;

    if (m_IsStreamRunning)
    {
        buf.m.userptr = (long unsigned int)m_UserBufferContainerList[m_nFrameBufferIndex]->pBuffer;
        buf.length = m_UserBufferContainerList[m_nFrameBufferIndex]->nBufferlength;
        result = read(m_nFileDescriptor, (uint8_t*)buf.m.userptr, buf.length);
        if (0 == result)
            m_nFrameBufferIndex++;
    }

    return result;
}

int FrameObserverRead::GetFrameData(v4l2_buffer &buf, uint8_t *&buffer, uint32_t &length)
{
    int result = -1;

    if (m_IsStreamRunning)
    {
        length = buf.length;
        buffer = (uint8_t*)buf.m.userptr;

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

int FrameObserverRead::CreateAllUserBuffer(uint32_t bufferCount, uint32_t bufferSize)
{
    int result = -1;

    if (bufferCount <= MAX_VIEWER_USER_BUFFER_COUNT)
    {
        // creates user defined buffer

        base::LocalMutexLockGuard guard(m_UsedBufferMutex);

        Logger::LogEx("FrameObserverUSER::CreateUserBuffer VIDIOC_REQBUFS OK");
        emit OnMessage_Signal("FrameObserverUSER::CreateUserBuffer: VIDIOC_REQBUFS OK.");

        // create local buffer container
        m_UserBufferContainerList.resize(bufferCount);

        if (m_UserBufferContainerList.size() != bufferCount)
        {
            Logger::LogEx("FrameObserverUSER::CreateUserBuffer buffer container error");
            emit OnError_Signal("FrameObserverUSER::CreateUserBuffer: buffer container error.");
            return -1;
        }

        // get the length and start address of each of the 4 buffer structs and assign the user buffer addresses
        for (unsigned int x = 0; x < m_UserBufferContainerList.size(); ++x)
        {
            UserBuffer* pTmpBuffer = new UserBuffer;
            pTmpBuffer->nBufferlength = bufferSize;
            m_RealPayloadSize = pTmpBuffer->nBufferlength;
            pTmpBuffer->pBuffer = new uint8_t[bufferSize];

            if (!pTmpBuffer->pBuffer)
            {
                delete pTmpBuffer;
                Logger::LogEx("FrameObserverUSER::CreateUserBuffer buffer creation error");
                emit OnError_Signal("FrameObserverUSER::CreateUserBuffer: buffer creation error.");
                m_UserBufferContainerList.resize(0);
                return -1;
            }
            else
                m_UserBufferContainerList[x] = pTmpBuffer;
        }

        result = 0;
    }

    return result;
}

int FrameObserverRead::QueueAllUserBuffer()
{
    int result = 0;

    return result;
}

int FrameObserverRead::QueueSingleUserBuffer(const int index)
{
    int result = 0;

    return result;
}

int FrameObserverRead::DeleteAllUserBuffer()
{
    int result = 0;

    // free all internal buffers
    // creates user defined buffer

    {
        base::LocalMutexLockGuard guard(m_UsedBufferMutex);

        // delete all user buffer
        for (unsigned int x = 0; x < m_UserBufferContainerList.size(); x++)
        {
            if (0 != m_UserBufferContainerList[x]->pBuffer)
                delete [] m_UserBufferContainerList[x]->pBuffer;
            if (0 != m_UserBufferContainerList[x])
                delete m_UserBufferContainerList[x];
        }

        m_UserBufferContainerList.resize(0);
    }

    return result;
}

/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        Thread.cpp

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

#include "Thread.h"


namespace base {

Thread::Thread()
{
}

Thread::~Thread(void)
{
}

void *Thread::StartThread(THREAD_START_ROUTINE threadStartRoutine, void* params)
{
    pthread_create(&m_ThreadID, NULL, threadStartRoutine, params);
    return (void*)m_ThreadID;
}

void *Thread::GetThreadID()
{
    return (void*)m_ThreadID;
}

uint32_t Thread::Join()
{
    uint32_t result;
    result = pthread_join(m_ThreadID, NULL);
    return result;
}

uint32_t Thread::JoinTimed(int msec)
{
    uint32_t  result;

    if (0 >= msec)
        return -1;

    struct timespec ts;
    ts.tv_sec = time(NULL);
    ts.tv_sec += msec / 1000;
    ts.tv_nsec = (msec - ((msec / 1000) * 1000)) * 1000;
    result = pthread_timedjoin_np(m_ThreadID, NULL, &ts);

    return result;
}

} // namespace base

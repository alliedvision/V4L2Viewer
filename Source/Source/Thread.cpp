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

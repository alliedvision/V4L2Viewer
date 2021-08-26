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


#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/errno.h>

typedef void *(*THREAD_START_ROUTINE)(void *);


namespace base {

class Thread
{
public:
    Thread();
    ~Thread(void);

    // This function starts thread
    void *StartThread(THREAD_START_ROUTINE threadStartRoutine, void* params);
    // This function returns thread id
    void *GetThreadID();
    // This function joins thread
    uint32_t Join();
    // This function joins thread with a delay
    uint32_t JoinTimed(int msec);

private:
    pthread_t m_ThreadID;
};

} // namespace base

#endif // THREAD_H


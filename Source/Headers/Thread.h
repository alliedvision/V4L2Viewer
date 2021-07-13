/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        Thread.h

Description: This class is responsible for application threading, it contains
             description of how the thread object looks like.

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


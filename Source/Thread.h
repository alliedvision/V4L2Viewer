/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        Thread.h

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

#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>

#ifdef _WIN32       /* Windows */
#include <Windows.h>
#else               /* Linux */
#include <pthread.h>
#include <sys/time.h>
#include <sys/errno.h>
#ifndef LPTHREAD_START_ROUTINE
typedef void *(*LPTHREAD_START_ROUTINE)(void *);
#endif // LPTHREAD_START_ROUTINE
#endif


namespace AVT {
namespace BaseTools {

#ifdef __APPLE__
    int pthread_timedjoin_np_avt(pthread_t td, void **res, struct timespec *ts);
#endif

    class Thread
    {
    public:
        Thread();
        ~Thread(void);

        void *StartThread(LPTHREAD_START_ROUTINE pfct, void* params);
        void *GetThreadID();
        uint32_t Join();
        uint32_t JoinTimed(int msec );

    private:
#ifdef _WIN32       /* Windows */
        HANDLE    m_ThreadID;
#else               /* Linux */
        pthread_t m_ThreadID;
#endif
    };

} // namespace BaseTools
} // namespace AVT

#endif // THREAD_H


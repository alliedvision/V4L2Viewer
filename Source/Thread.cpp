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


namespace AVT {
namespace BaseTools {

#ifdef __APPLE__
    struct arguments {
        int s_threadCanceled;
        int s_joined;
        pthread_t s_threadID;
        pthread_mutex_t s_mutex;
        pthread_cond_t s_condition;
        void **s_res;
    };

    static void *waiter(void *ap)
    {
        struct arguments *args = (struct arguments*)ap;
        pthread_join(args->s_threadID, args->s_res);
        pthread_mutex_lock(&args->s_mutex);
        pthread_mutex_unlock(&args->s_mutex);
        if (!args->s_threadCanceled)
            args->s_joined = 1;
        pthread_cond_signal(&args->s_condition);
        return 0;
    }

    int pthread_timedjoin_np_avt(pthread_t threadID, void **res, struct timespec *ts)
    {
        pthread_t localThreadID;
        int ret;
        struct arguments localArgs = { .s_joined = 0, .s_threadCanceled = 0, .s_threadID = threadID, .s_res = res };

        pthread_mutex_init(&localArgs.s_mutex, 0);
        pthread_cond_init(&localArgs.s_condition, 0);
        pthread_mutex_lock(&localArgs.s_mutex);

        ret = pthread_create(&localThreadID, 0, waiter, &localArgs);
        if (ret)
            return 0;

        do
            ret = pthread_cond_timedwait(&localArgs.s_condition, &localArgs.s_mutex, ts);
        while (localArgs.s_joined != 1 && ret != ETIMEDOUT);

        if (ret == ETIMEDOUT)
            localArgs.s_threadCanceled = 1;
        pthread_cancel(localThreadID);
        pthread_mutex_unlock(&localArgs.s_mutex);
        pthread_join(localThreadID, 0);

        pthread_cond_destroy(&localArgs.s_condition);
        pthread_mutex_destroy(&localArgs.s_mutex);

        return localArgs.s_joined ? 0 : ret;
    }
#endif /* __APPLE__ */

    Thread::Thread()
    {
    }

    Thread::~Thread(void)
    {
    }

    void *Thread::StartThread(LPTHREAD_START_ROUTINE pfct, void* params)
    {
#ifdef _WIN32       /* Windows */
        m_ThreadID = CreateThread((LPSECURITY_ATTRIBUTES)NULL,                          // No security.
                                  (DWORD)0,							                    // Same stack size.
                                  pfct,		                                            // Thread procedure.
                                  (LPVOID)params,	                                        // Pointer the private data.
                                  (DWORD)0,							                    // Start immediately.
                                  NULL);								                    // Thread ID.
#else               /* Linux */
        pthread_create(&m_ThreadID, NULL, pfct, params);
#endif
        return (void*)m_ThreadID;
    }

    void *Thread::GetThreadID()
    {
        return (void*)m_ThreadID;
    }

    uint32_t Thread::Join()
    {
        uint32_t result;
#ifdef _WIN32       /* Windows */
        result = WaitForMultipleObjects(1, &m_ThreadID, TRUE, -1);
#else               /* Linux */
        result = pthread_join(m_ThreadID, NULL);
#endif
        return result;
    }

    uint32_t Thread::JoinTimed(int msec)
    {
        uint32_t  result;

        if (0 >= msec)
            return -1;

#ifdef _WIN32                                                           /* Windows */
        result = WaitForMultipleObjects(1, &m_ThreadID, TRUE, msec);
#elif defined (__APPLE__)                                               /* Apple */
        struct timespec ts;
        ts.tv_sec = time(NULL);
        ts.tv_sec += msec / 1000;
        ts.tv_nsec = (msec - ((msec / 1000) * 1000)) * 1000;
        result = pthread_timedjoin_np_avt(m_ThreadID, NULL, &ts);
#elif defined (__GNUC__) && (__GNUC__ >= 4) && defined (__ELF__)        /* Linux */
        struct timespec ts;
        ts.tv_sec = time(NULL);
        ts.tv_sec += msec / 1000;
        ts.tv_nsec = (msec - ((msec / 1000) * 1000)) * 1000;
        result = pthread_timedjoin_np(m_ThreadID, NULL, &ts);
#else
        #error Unknown platform, file needs adaption
#endif
        return result;
    }

} // namespace BaseTools
} // namespace AVT

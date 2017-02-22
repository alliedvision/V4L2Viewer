#ifndef THREAD_INCLUDE
#define THREAD_INCLUDE

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

}} /* namespace AVT::BaseTools */

#endif /* THREAD_INCLUDE */



#ifndef MUTEX_INCLUDE
#define MUTEX_INCLUDE

#ifdef _WIN32       /* Windows */
#include <Windows.h>
#else               /* Linux */
#include <pthread.h>
#endif

namespace AVT {
namespace BaseTools {

    class LocalMutex
    {
    public:
        LocalMutex()
        {
    #ifdef _WIN32       /* Windows */
            InitializeCriticalSection(&m_Mutex);
    #else               /* Linux */
            pthread_mutex_init(&m_Mutex, NULL);
    #endif
        }
        virtual ~LocalMutex()
        {
    #ifdef _WIN32       /* Windows */
            DeleteCriticalSection(&m_Mutex);
    #else               /* Linux */
            pthread_mutex_destroy(&m_Mutex);
    #endif
        }

        void Lock() 
        { 
    #ifdef _WIN32       /* Windows */
            EnterCriticalSection(&m_Mutex); 
    #else               /* Linux */
            pthread_mutex_lock(&m_Mutex); 
    #endif
        }
        void Unlock() 
        { 
    #ifdef _WIN32       /* Windows */
            LeaveCriticalSection(&m_Mutex);
    #else               /* Linux */
            pthread_mutex_unlock(&m_Mutex);
    #endif
        }

    #ifdef _WIN32       /* Windows */
	    CRITICAL_SECTION m_Mutex; 
    #else               /* Linux */
        pthread_mutex_t  m_Mutex; 
    #endif

    };

    class AutoLocalMutex
    {
    public:
        AutoLocalMutex(LocalMutex &mutex)
            : m_Mutex(mutex)
        {
            m_Mutex.Lock();
        }

        virtual ~AutoLocalMutex()
        {
            m_Mutex.Unlock();
        }

    private:
        LocalMutex &m_Mutex;
    };

}} /* namespace AVT::BaseTools */

#endif /* MUTEX_INCLUDE */
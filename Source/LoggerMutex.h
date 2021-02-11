/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        LoggerMutex.h

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

#ifndef LOCALMUTEX_H
#define LOCALMUTEX_H

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

} // namespace BaseTools
} // namespace AVT

#endif // LOCALMUTEX_H

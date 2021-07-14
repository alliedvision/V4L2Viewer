/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        LocalMutex.h

Description: This class describes mutex and provides functions for lock and
             unlock.

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

#include <pthread.h>

namespace base {

class LocalMutex
{
public:
    LocalMutex()
    {
        pthread_mutex_init(&m_Mutex, NULL);
    }
    virtual ~LocalMutex()
    {
        pthread_mutex_destroy(&m_Mutex);
    }

    // This function locks mutex
    void Lock()
    {
        pthread_mutex_lock(&m_Mutex);
    }
    // This function unlocks mutex
    void Unlock()
    {
        pthread_mutex_unlock(&m_Mutex);
    }

    pthread_mutex_t  m_Mutex;
};

} // namespace base

#endif // LOCALMUTEX_H

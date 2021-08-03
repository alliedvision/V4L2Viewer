/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AutoReader.h

  Description: This class is responsible for handling reading back auto
               controls.

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

#ifndef AUTOREADER_H
#define AUTOREADER_H

#include <QObject>
#include <QThread>
#include "AutoReaderWorker.h"


class AutoReader : public QObject {

    Q_OBJECT

public:
    AutoReader(QObject *parent=nullptr);
    ~AutoReader();
    // This function emits starting thread signal
    void StartThread();
    // This function emits stopping thread signal
    void StopThread();
    // This function moves object to thread and starts it
    void MoveToThreadAndStart();
    // This fucntion releases working thread
    void DeleteThread();
    // This function returns worker
    //
    // Returns:
    // (AutoReaderWorker *) - auto reader worker
    AutoReaderWorker *GetAutoReaderWorker();

signals:
    // This signal sends info about stopping timer in the worker thread
    void StopTimer();
    // This signal sends info about starting timer in the worker thread
    void StartTimer();

private:
    // Worker's thread
    QThread *m_pThread;
    // Worker's object
    AutoReaderWorker *m_pAutoReaderWorker;
};

#endif // AUTOREADER_H

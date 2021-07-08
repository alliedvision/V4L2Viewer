/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AutoReaderWorker.h

  Description: This class is a worker for the AutoReader object.

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

#ifndef AUTOREADERWORKER_H
#define AUTOREADERWORKER_H

#include <QObject>
#include <QTimer>

class AutoReaderWorker : public QObject {
    Q_OBJECT

public:
    AutoReaderWorker(QObject *parent = nullptr);
    ~AutoReaderWorker();

signals:
    // This signal emits info to read control's value
    void ReadSignal();

public slots:
    // This slot function preparing the thread and connects QTimer with slot
    void Process();
    // This slot function starts timer
    void StartProcess();
    // This slot function stops timer
    void StopProcess();

private slots:
    // This slot function is called by qtimer and emits read signal
    void ReadData();

private:
    // Timer object which is started and stopped in this class, which is a thread worker.
    QTimer *m_pTimer;
};

#endif // AUTOREADERWORKER_H

/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AutoReader.cpp

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

#include "AutoReader.h"

AutoReader::AutoReader(QObject *parent) : QObject(parent)
{
    m_pThread = new QThread;
    m_pAutoReaderWorker = new AutoReaderWorker();
    connect(this, SIGNAL(StartTimer()), m_pAutoReaderWorker, SLOT(StartProcess()));
    connect(this, SIGNAL(StopTimer()), m_pAutoReaderWorker, SLOT(StopProcess()));
    connect(m_pThread, SIGNAL(started()), m_pAutoReaderWorker, SLOT(Process()));
}

AutoReader::~AutoReader()
{
    emit StopTimer();
    m_pThread->quit();
    m_pThread->wait();
    delete m_pThread;
    m_pThread = nullptr;
    delete m_pAutoReaderWorker;
    m_pAutoReaderWorker = nullptr;
    qDebug() << "AutoReader Deleted";
}

void AutoReader::StartThread()
{
    emit StartTimer();
}

void AutoReader::StopThread()
{
    emit StopTimer();
}

void AutoReader::MoveToThreadAndStart()
{
    m_pAutoReaderWorker->moveToThread(m_pThread);
    m_pThread->start();
}

AutoReaderWorker *AutoReader::GetAutoReaderWorker()
{
    return m_pAutoReaderWorker;
}

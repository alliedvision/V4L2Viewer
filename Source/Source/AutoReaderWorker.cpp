/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AutoReaderWorker.cpp

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

#include "AutoReaderWorker.h"

AutoReaderWorker::AutoReaderWorker(QObject *parent) : QObject(parent), m_pTimer(nullptr)
{

}

AutoReaderWorker::~AutoReaderWorker()
{
    if (m_pTimer->isActive())
    {
        m_pTimer->stop();
    }
    delete m_pTimer;
    m_pTimer = nullptr;
}

void AutoReaderWorker::Process()
{
    if (m_pTimer == nullptr)
    {
        m_pTimer = new QTimer();
        connect(m_pTimer, SIGNAL(timeout()), this, SLOT(ReadData()));
        m_pTimer->setInterval(1000);
    }
}

void AutoReaderWorker::StartProcess()
{
    m_pTimer->start();
}

void AutoReaderWorker::StopProcess()
{
    m_pTimer->stop();
}

void AutoReaderWorker::ReadData()
{
    emit ReadSignal();
}

/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2021 Allied Vision Technologies GmbH

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.  */


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

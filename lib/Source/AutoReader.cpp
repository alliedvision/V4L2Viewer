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

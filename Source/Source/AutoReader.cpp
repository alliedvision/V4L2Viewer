#include "AutoReader.h"
#include <QDebug>

AutoReader::AutoReader(QObject *parent) : QObject(parent)
{
    m_pThread = new QThread;
    m_pAutoReaderWorker = new AutoReaderWorker();
    connect(this, SIGNAL(StartTimer()), m_pAutoReaderWorker, SLOT(StartProcess()));
    connect(this, SIGNAL(StopTimer()), m_pAutoReaderWorker, SLOT(StopProcess()));
    connect(m_pThread, SIGNAL(started()), m_pAutoReaderWorker, SLOT(Process()));
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

void AutoReader::DeleteThread()
{
    emit StopTimer();
    m_pThread->quit();
    m_pThread->deleteLater();
}

AutoReaderWorker *AutoReader::GetAutoReaderWorker()
{
    return m_pAutoReaderWorker;
}

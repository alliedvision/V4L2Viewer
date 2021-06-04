#include "AutoReaderWorker.h"
#include <QDebug>

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
    qDebug()<<"ReadData From thread";
    emit ReadSignal();
}

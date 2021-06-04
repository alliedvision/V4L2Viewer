#ifndef AUTOREADER_H
#define AUTOREADER_H

#include <QObject>
#include <QThread>
#include "AutoReaderWorker.h"


class AutoReader : public QObject {

    Q_OBJECT

public:
    AutoReader(QObject *parent=nullptr);

    void StartThread();
    void StopThread();
    void MoveToThreadAndStart();
    void DeleteThread();
    AutoReaderWorker *GetAutoReaderWorker();

signals:
    void StopTimer();
    void StartTimer();

private:
    QThread *m_pThread;
    AutoReaderWorker *m_pAutoReaderWorker;
};

#endif // AUTOREADER_H

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
    void ReadSignal();

public slots:
    void Process();
    void StartProcess();
    void StopProcess();

private slots:
    void ReadData();

private:
    QTimer *m_pTimer;
};

#endif // AUTOREADERWORKER_H

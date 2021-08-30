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


#ifndef AUTOREADERWORKER_H
#define AUTOREADERWORKER_H

#include <QObject>
#include <QTimer>
#include <QDebug>

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

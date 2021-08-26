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


#ifndef AUTOREADER_H
#define AUTOREADER_H

#include <QObject>
#include <QThread>
#include "AutoReaderWorker.h"


class AutoReader : public QObject {

    Q_OBJECT

public:
    AutoReader(QObject *parent=nullptr);
    ~AutoReader();
    // This function emits starting thread signal
    void StartThread();
    // This function emits stopping thread signal
    void StopThread();
    // This function moves object to thread and starts it
    void MoveToThreadAndStart();
    // This fucntion releases working thread
    void DeleteThread();
    // This function returns worker
    //
    // Returns:
    // (AutoReaderWorker *) - auto reader worker
    AutoReaderWorker *GetAutoReaderWorker();

signals:
    // This signal sends info about stopping timer in the worker thread
    void StopTimer();
    // This signal sends info about starting timer in the worker thread
    void StartTimer();

private:
    // Worker's thread
    QThread *m_pThread;
    // Worker's object
    AutoReaderWorker *m_pAutoReaderWorker;
};

#endif // AUTOREADER_H

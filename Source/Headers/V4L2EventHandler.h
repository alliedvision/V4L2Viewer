/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2022 Allied Vision Technologies GmbH

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


#ifndef V4L2EVENTREADER_H
#define V4L2EVENTREADER_H

#include <QThread>

class V4L2EventHandler : public QThread {
    Q_OBJECT
public:
    explicit V4L2EventHandler(int fd);
    virtual ~V4L2EventHandler();

    // This function enables the reception of changes for the control id
    void SubscribeControl(int id);

    // This function disables the reception of changes for the control id
    void UnsubscribeControl(int id);

    // This function stops the event handling and the underlying thread
    void Stop();
signals:
    // This signal is emitted when the value of a control has changed and contains the new value
    void ControlChanged(int cid,v4l2_event_ctrl value);
protected:
    // Function of the thread
    void run() override;
private:
    // File descriptor of the camera
    int m_Fd;
    int m_eventFd;
};

#endif //V4L2EVENTREADER_H

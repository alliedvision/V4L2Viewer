/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2021 - 2025 Allied Vision Technologies GmbH

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
#include <QVector>
#include <linux/videodev2.h>

#define AVT_V4L2_EVENT_PIXELFORMAT_CHANGE V4L2_EVENT_PRIVATE_START+1

class V4L2EventHandler : public QThread {
    Q_OBJECT
public:
    explicit V4L2EventHandler(const std::vector<int>  & fds);
    virtual ~V4L2EventHandler();

    // This function enables the reception of changes for the control id
    void SubscribeControl(int id);

    // This function subscribes to custom pixel format change envents
    void SubscribePixelFormatChange();

    // This function disables the reception of changes for the control id
    void UnsubscribeControl(int id);

    // This function disables the reception of changes for the custom pixel format change event
    void UnsubscribePixelFormatChange();

    // This function stops the event handling and the underlying thread
    void Stop();
signals:
    // This signal is emitted when the value of a control has changed and contains the new value
    void ControlChanged(int cid,v4l2_event_ctrl value);
    // This signal is emitted when the driver detects a pixel format change (reverse xy is changed while a bayer pixel format is selected)
    void PixelFormatChanged();
protected:
    // Function of the thread
    void run() override;
private:
    // File descriptor of the camera
    std::vector<int> m_Fds;
    int m_eventFd;
};

#endif //V4L2EVENTREADER_H

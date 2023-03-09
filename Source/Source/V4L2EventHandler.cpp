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

#include <iostream>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <linux/videodev2.h>
#include <sys/eventfd.h>
#include <unistd.h>


#include "V4L2EventHandler.h"

V4L2EventHandler::V4L2EventHandler(const std::vector<int>  & fds) : m_Fds(fds)
{

    m_eventFd = eventfd(0,0);
}

V4L2EventHandler::~V4L2EventHandler()
{
}

void V4L2EventHandler::SubscribeControl(int id)
{
    for (const auto & fd : m_Fds)
    {
        v4l2_event_subscription subscription = {0};
        subscription.id = id;
        subscription.type = V4L2_EVENT_CTRL;
        subscription.flags = V4L2_EVENT_SUB_FL_ALLOW_FEEDBACK;
        ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &subscription);
    }
}

void V4L2EventHandler::UnsubscribeControl(int id)
{
    for (const auto & fd : m_Fds)
    {
        v4l2_event_subscription subscription = {0};
        subscription.id = id;
        subscription.type = V4L2_EVENT_CTRL;
        ioctl(fd, VIDIOC_UNSUBSCRIBE_EVENT, &subscription);
    }
}

void V4L2EventHandler::run()
{
    while (!isInterruptionRequested())
    {
        std::vector<pollfd> pfds;

        for (const auto fd : m_Fds)
        {
            pollfd pfd = {
                .fd = fd,
                .events = POLLPRI,
            };

            pfds.push_back(pfd);
        }

        pollfd event_pfd = {
            .fd = m_eventFd,
            .events = POLLIN,
        };

        pfds.push_back(event_pfd);

        int ret = poll(pfds.data(),pfds.size(),-1);
        if (ret > 0)
        {
            for (const auto & pfd : pfds)
            {
                if (pfd.revents & POLLERR)
                {
                    auto const idx = &pfd - pfds.data();
                    if (idx < m_Fds.size())
                    {
                        m_Fds.erase(m_Fds.begin() + idx);
                    }
                }

                if (pfd.revents & POLLPRI)
                {
                    v4l2_event event = {0};

                    int error = ioctl(pfd.fd,VIDIOC_DQEVENT,&event);

                    if (!error)
                    {
                        if (event.type == V4L2_EVENT_CTRL)
                        {
                            emit ControlChanged(event.id,event.u.ctrl);
                        }
                    }
                }
            }
        }
    }
    eventfd_t value;
    eventfd_read(m_eventFd,&value);
}

void V4L2EventHandler::Stop()
{
    requestInterruption();
    eventfd_write(m_eventFd,1);
    eventfd_write(m_eventFd,0xfffffffffffffffe);
    eventfd_t value;
    eventfd_read(m_eventFd,&value);

	for (const auto & fd : m_Fds)
    {
        v4l2_event_subscription subscription = {0};
        subscription.type = V4L2_EVENT_ALL;
        ioctl(fd,VIDIOC_UNSUBSCRIBE_EVENT,&subscription);
    }
}


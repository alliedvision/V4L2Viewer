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


#include "CameraObserver.h"
#include "Logger.h"

#include <errno.h>
#include <fcntl.h>
#include <IOHelper.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MAX_DEVICE_COUNT 10

CameraObserver::CameraObserver(void)
    : m_bTerminate(false)
    , m_bAbort(false)
{

}


CameraObserver::~CameraObserver(void)
{
    Stop();
}

void CameraObserver::Start()
{
    start();
}

void CameraObserver::Stop()
{
    // stop the internal processing thread and wait until the thread is really stopped
    m_bAbort = true;

    // wait until the thread is stopped
    while (isRunning())
        QThread::msleep(10);
}

void CameraObserver::SetTerminateFlag()
{
    m_bTerminate = true;
}

// Do the work within this thread
void CameraObserver::run()
{
    while (!m_bAbort)
    {
        CheckDevices();
        CheckSubDevices();

        QThread::msleep(1000);
    }
}

int CameraObserver::CheckDevices()
{
    uint32_t deviceCount = 0;
    QString deviceName;
    int fileDiscriptor = -1;
    std::map<int, std::string>    deviceList;

    do
    {
        deviceName.sprintf("/dev/video%d", deviceCount);

        if ((fileDiscriptor = open(deviceName.toStdString().c_str(), O_RDWR)) == -1)
        {
            LOG_EX("CameraObserver::CheckDevices open %s failed", deviceName.toLatin1().data());
            emit OnCameraError_Signal("CameraObserver::CheckDevices: open " + deviceName + " failed");
        }
        else
        {
            v4l2_capability cap;

            LOG_EX("CameraObserver::CheckDevices open %s found", deviceName.toLatin1().data());
            emit OnCameraMessage_Signal("CameraObserver::CheckDevices: open " + deviceName + " found");

            // query device capabilities
            if (-1 == iohelper::xioctl(fileDiscriptor, VIDIOC_QUERYCAP, &cap))
            {
                LOG_EX("CameraObserver::CheckDevices %s is no V4L2 device", deviceName.toLatin1().data());
                emit OnCameraError_Signal("CameraObserver::CheckDevices: " + deviceName + " is no V4L2 device");
            }
            else
            {
                if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
                {
                    LOG_EX("CameraObserver::CheckDevices %s is a single-plane video capture device", deviceName.toLatin1().data());
                }
                else if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                {
                    LOG_EX("CameraObserver::CheckDevices %s is a multi-plane video capture device", deviceName.toLatin1().data());
                }
                else
                {
                    LOG_EX("CameraObserver::CheckDevices %s is no video capture device", deviceName.toLatin1().data());
                    emit OnCameraError_Signal("CameraObserver::CheckDevices: " + deviceName + " is no video capture device");
                }
            }


            if (-1 == close(fileDiscriptor))
            {
                LOG_EX("CameraObserver::CheckDevices close %s failed", deviceName.toLatin1().data());
                emit OnCameraError_Signal("CameraObserver::CheckDevices: close " + deviceName + " failed");
            }
            else
            {
                deviceList[deviceCount] = deviceName.toStdString();

                deviceCount++;

                LOG_EX("CameraObserver::CheckDevices close %s OK", deviceName.toLatin1().data());
                emit OnCameraMessage_Signal("CameraObserver::CheckDevices: close " + deviceName + " OK");
            }
        }
    }
    while(++deviceCount < MAX_DEVICE_COUNT);

    for (unsigned int i=0; i<deviceList.size(); i++)
    {
        bool found = false;

        for (unsigned int ii=0; ii<m_DeviceList.size(); ii++)
        {
            if (deviceList[i] == m_DeviceList[ii])
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            unsigned int ii = 0;
            for (ii=0; ii<m_DeviceList.size(); ii++)
            {
                bool found = false;
                for (std::map<int, std::string>::iterator iii=m_DeviceList.begin(); iii!=m_DeviceList.end(); iii++)
                {
                    if (iii->first == static_cast<int>(ii) )
                    {
                        found = true;
                        break;
                    }
                }
                if (found == false)
                    break;
            }
            m_DeviceList[ii] = deviceList[i].c_str();
            emit OnCameraListChanged_Signal(UpdateTriggerPluggedIn, 0, ii, m_DeviceList[ii].c_str(), "");
        }
    }

    for (unsigned int i=0; i<m_DeviceList.size(); i++)
    {
        bool found = false;

        for (unsigned int ii=0; ii<deviceList.size(); ii++)
        {
            if (deviceList[ii] == m_DeviceList[i])
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            m_DeviceList.erase(i);
            emit OnCameraListChanged_Signal(UpdateTriggerPluggedOut, 0, i, m_DeviceList[i].c_str(), "");
        }
    }

    return 0;
}

// Callbacks

void CameraObserver::OnDeviceReady(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData)
{
    if (!m_bTerminate)
    {
        emit OnCameraListChanged_Signal(UpdateTriggerPluggedIn, cardNumber, deviceID, "", "");
    }
}

void CameraObserver::OnDeviceRemoved(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData)
{
    if (!m_bTerminate)
    {
        emit OnCameraListChanged_Signal(UpdateTriggerPluggedOut, cardNumber, deviceID, "", "");
    }
}

void CameraObserver::OnMessage(const char *text, void *pPrivateData)
{
    if (!m_bTerminate)
    {
        LOG_EX("CameraObserver: message = '%s'", text);
    }
}

int CameraObserver::CheckSubDevices()
{
    uint32_t subDeviceCount = 0;
    QString subDeviceName;
    int fileDiscriptor = -1;
    std::map<int, std::string>    subDeviceList;

    do
    {
        subDeviceName.sprintf("/dev/video%d", subDeviceCount);

        if ((fileDiscriptor = open(subDeviceName.toStdString().c_str(), O_RDWR)) == -1)
        {
            LOG_EX("CameraObserver::CheckSubDevices open %s failed", subDeviceName.toLatin1().data());
            emit OnSubDeviceError_Signal("CameraObserver::CheckSubDevices: open " + subDeviceName + " failed");
        }
        else
        {
            v4l2_capability cap;

            LOG_EX("CameraObserver::CheckSubDevices open %s found", subDeviceName.toLatin1().data());
            emit OnCameraMessage_Signal("CameraObserver::CheckSubDevices: open " + subDeviceName + " found");

            // query device capabilities
            if (-1 == iohelper::xioctl(fileDiscriptor, VIDIOC_QUERYCAP, &cap))
            {
                LOG_EX("CameraObserver::CheckSubDevices %s is no V4L2 device", subDeviceName.toLatin1().data());
                emit OnSubDeviceError_Signal("CameraObserver::CheckSubDevices: " + subDeviceName + " is no V4L2 device");
            }
            else
            {
                if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
                {
                    LOG_EX("CameraObserver::CheckSubDevices %s is a single-plane video capture device", subDeviceName.toLatin1().data());
                }
                else if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                {
                    LOG_EX("CameraObserver::CheckSubDevices %s is a multi-plane video capture device", subDeviceName.toLatin1().data());
                }
                else
                {
                    LOG_EX("CameraObserver::CheckSubDevices %s is no video capture device", subDeviceName.toLatin1().data());
                    emit OnSubDeviceError_Signal("CameraObserver::CheckSubDevices: " + subDeviceName + " is no video capture device");
                }
            }


            if (-1 == close(fileDiscriptor))
            {
                LOG_EX("CameraObserver::CheckSubDevices close %s failed", subDeviceName.toLatin1().data());
                emit OnCameraError_Signal("CameraObserver::CheckSubDevices: close " + subDeviceName + " failed");
            }
            else
            {
                subDeviceList[subDeviceCount] = subDeviceName.toStdString();

                subDeviceCount++;

                LOG_EX("CameraObserver::CheckDevices close %s OK", subDeviceName.toLatin1().data());
                emit OnSubDeviceMessage_Signal("CameraObserver::CheckDevices: close " + subDeviceName + " OK");
            }
        }
    }
    while(++subDeviceCount < MAX_DEVICE_COUNT);

    for (unsigned int i=0; i<subDeviceList.size(); i++)
    {
        bool found = false;

        for (unsigned int ii=0; ii<m_SubDeviceList.size(); ii++)
        {
            if (subDeviceList[i] == m_SubDeviceList[ii])
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            unsigned int ii = 0;
            for (ii=0; ii<m_SubDeviceList.size(); ii++)
            {
                bool found = false;
                for (std::map<int, std::string>::iterator iii=m_SubDeviceList.begin(); iii!=m_SubDeviceList.end(); iii++)
                {
                    if (iii->first == static_cast<int>(ii) )
                    {
                        found = true;
                        break;
                    }
                }
                if (found == false)
                    break;
            }
            m_SubDeviceList[ii] = subDeviceList[i].c_str();
            emit OnSubDeviceListChanged_Signal(UpdateTriggerPluggedIn, 0, ii, m_SubDeviceList[ii].c_str(), "");
        }
    }

    for (unsigned int i=0; i<m_SubDeviceList.size(); i++)
    {
        bool found = false;

        for (unsigned int ii=0; ii<subDeviceList.size(); ii++)
        {
            if (subDeviceList[ii] == m_SubDeviceList[i])
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            m_SubDeviceList.erase(i);
            emit OnSubDeviceListChanged_Signal(UpdateTriggerPluggedOut, 0, i, m_SubDeviceList[i].c_str(), "");
        }
    }

    return 0;
}

// Callbacks

void CameraObserver::OnSubDeviceReady(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData)
{
    if (!m_bTerminate)
    {
        emit OnSubDeviceListChanged_Signal(UpdateTriggerPluggedIn, cardNumber, deviceID, "", "");
    }
}

void CameraObserver::OnSubDeviceRemoved(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData)
{
    if (!m_bTerminate)
    {
        emit OnSubDeviceListChanged_Signal(UpdateTriggerPluggedOut, cardNumber, deviceID, "", "");
    }
}

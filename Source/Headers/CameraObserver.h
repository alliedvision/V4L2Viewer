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


#ifndef CAMERAOBSERVER_H
#define CAMERAOBSERVER_H

#include <QImage>
#include <QObject>
#include <QSharedPointer>
#include <QThread>

#include <stdint.h>

#include <map>

enum UpdateTriggerType
{
    UpdateTriggerPluggedIn           = 0,
    UpdateTriggerPluggedOut          = 1,
    UpdateTriggerOpenStateChanged    = 3
};

class CameraObserver : public QThread
{
    Q_OBJECT

public:
    CameraObserver(void);
    virtual ~CameraObserver(void);

    // This function starts current thread
    void Start();
    // This function stops current thread
    void Stop();

    // This function sets terminate flag
    void SetTerminateFlag();

    // Implementation
    // Do the work within this thread
    virtual void run();

    // This function check for the devices, after finding
    // any, emits signal with the device information
    //
    // Returns:
    // (int) - result of operation
    int CheckDevices();

    // This function check for the sub-devices, after finding
    // any, emits signal with the sub-device information
    //
    // Returns:
    // (int) - result of operation
    int CheckSubDevices();

    // Callbacks

    // This function emits signal plugged in
    //
    // Parameters:
    // [in] (uint32_t) cardNumber
    // [in] (uint64_t) deviceID
    // [in] (const void *) pPrivateData
    virtual void OnDeviceReady(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData);
    // This function emits signal plugged out
    //
    // Parameters:
    // [in] (uint32_t) cardNumber
    // [in] (uint64_t) deviceID
    // [in] (const void *) pPrivateData
    virtual void OnDeviceRemoved(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData);

    // This function emits signal plugged in
    //
    // Parameters:
    // [in] (uint32_t) cardNumber
    // [in] (uint64_t) deviceID
    // [in] (const void *) pPrivateData
    virtual void OnSubDeviceReady(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData);
    // This function emits signal plugged out
    //
    // Parameters:
    // [in] (uint32_t) cardNumber
    // [in] (uint64_t) deviceID
    // [in] (const void *) pPrivateData
    virtual void OnSubDeviceRemoved(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData);


private:
    // Callbacks

    // This function logs message
    //
    // Parameters:
    // [in] (const char*) text
    // [in] (void *) pPrivateData
    virtual void OnMessage(const char *text, void *pPrivateData);

signals:
    // The camera list changed signal that passes the new camera and the its state directly
    void OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &, const QString &);
    // Event will be called on error
    void OnCameraError_Signal(const QString &text);
    // Event will be called on message
    void OnCameraMessage_Signal(const QString &text);

    // The sub-device list changed signal that passes the new camera and the its state directly
    void OnSubDeviceListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &, const QString &);
    // Event will be called on error
    void OnSubDeviceError_Signal(const QString &text);
    // Event will be called on message
    void OnSubDeviceMessage_Signal(const QString &text);

private:
    bool                                        m_bTerminate;
    // Variable to abort the running thread
    bool                                        m_bAbort;
    std::map<int, std::string>                  m_DeviceList;
    std::map<int, std::string>                  m_SubDeviceList;
};

#endif // CAMERAOBSERVER_H


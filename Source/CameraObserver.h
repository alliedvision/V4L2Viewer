/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        CameraObserver.h

  Description:

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

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

    void Start();
    void Stop();

    void SetTerminateFlag();

    // Implementation
    // Do the work within this thread
    virtual void run();

    int CheckDevices();

    // Callbacks

    // Device
    virtual void OnDeviceReady(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData);
    virtual void OnDeviceRemoved(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData);


private:
    // Callbacks

    // Messages
    virtual void OnMessage(const char *text, void *pPrivateData);

signals:
    // The camera list changed signal that passes the new camera and the its state directly
    void OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &, const QString &);
    // Event will be called on error
    void OnCameraError_Signal(const QString &text);
    // Event will be called on message
    void OnCameraMessage_Signal(const QString &text);

private:
    bool                                        m_bTerminate;

    // Variable to abort the running thread
    bool                                        m_bAbort;

    std::map<int, std::string>                  m_DeviceList;

};

#endif // CAMERAOBSERVER_H


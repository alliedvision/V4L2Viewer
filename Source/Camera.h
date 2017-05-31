/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        CameraObserver.h

  Description: The camera observer that is used for notifications
               regarding a change in the camera list.

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

#ifndef AVT_VMBAPI_EXAMPLES_CAMERA
#define AVT_VMBAPI_EXAMPLES_CAMERA

#include <QObject>
#include "FrameObserver.h"
#include "CameraObserver.h"


namespace AVT {
namespace Tools {
namespace Examples {

class Camera : public QObject
{
    Q_OBJECT

public:
    Camera();
    virtual ~Camera();

    int OpenDevice(std::string &deviceName, bool blockingMode, bool mmapBuffer);
    int CloseDevice();

    int DeviceDiscoveryStart();
    int DeviceDiscoveryStop();
    int StartStreamChannel(uint32_t pixelformat, uint32_t payloadsize, uint32_t width, uint32_t height, uint32_t bytesPerLine, void *pPrivateData);
    int StopStreamChannel();
    
    int ReadPayloadsize(uint32_t &payloadsize);
    int ReadFrameSize(uint32_t &width, uint32_t &height);
    int SetFrameSize(uint32_t width, uint32_t height);
    int ReadWidth(uint32_t &width);
    int SetWidth(uint32_t width);
    int ReadHeight(uint32_t &height);
    int SetHeight(uint32_t height);
    int ReadPixelformat(uint32_t &pixelformat, uint32_t &bytesPerLine, QString &pfText);
    int SetPixelformat(uint32_t pixelformat, QString pfText);
    int ReadFormats();
    int ReadGain(uint32_t &gain);
    int SetGain(uint32_t gain);
    int ReadAutoGain(bool &autogain);
    int SetAutoGain(bool autogain);
    int ReadExposure(uint32_t &exposure);
    int ReadExposureAbs(uint32_t &exposure);
    int SetExposure(uint32_t exposure);
    int SetExposureAbs(uint32_t exposure);
    int ReadAutoExposure(bool &autoexposure);
    int SetAutoExposure(bool autoexposure);
    int ReadGamma(uint32_t &value);
    int SetGamma(uint32_t value);
    int ReadReverseX(uint32_t &value);
    int SetReverseX(uint32_t value);
    int ReadReverseY(uint32_t &value);
    int SetReverseY(uint32_t value);
    int ReadSharpness(uint32_t &value);
    int SetSharpness(uint32_t value);
    int ReadBrightness(uint32_t &value);
    int SetBrightness(uint32_t value);
    int ReadContrast(uint32_t &value);
    int SetContrast(uint32_t value);
    int ReadSaturation(uint32_t &value);
    int SetSaturation(uint32_t value);
    int ReadHue(uint32_t &value);
    int SetHue(uint32_t value);
    int SetContinousWhiteBalance(bool flag);
    int DoWhiteBalanceOnce();
    int ReadRedBalance(uint32_t &value);
    int SetRedBalance(uint32_t value);
    int ReadBlueBalance(uint32_t &value);
    int SetBlueBalance(uint32_t value);
    int ReadFramerate(uint32_t &value);
    int SetFramerate(uint32_t value);
    int SetControl(uint32_t value, uint32_t controlID, const char *functionName, const char* controlName);
    int ReadControl(uint32_t &value, uint32_t controlID, const char *functionName, const char* controlName);
	
    int StartStreaming();
    int StopStreaming();

    int CreateUserBuffer(uint32_t bufferCount, uint32_t bufferSize);
    int QueueAllUserBuffer();
    int QueueSingleUserBuffer(const int index);
    int DeleteUserBuffer();
    
    // Info
    int GetCameraDriverName(uint32_t index, std::string &strText);
    int GetCameraDeviceName(uint32_t index, std::string &strText);
    int GetCameraBusInfo(uint32_t index, std::string &strText);
    int GetCameraDriverVersion(uint32_t index, std::string &strText);
    int GetCameraCapabilities(uint32_t index, std::string &strText);
    
    // Statistics
    unsigned int GetReceivedFramesCount();
    unsigned int GetDroppedFramesCount();

    // Recording
    void SetRecording(bool start);
    void DisplayStepBack();
    void DisplayStepForw();
    void DeleteRecording();

    // Misc
    void SwitchFrameTransfer2GUI(bool showFrames);

private:
    std::string				m_DeviceName;
    int 				m_nFileDescriptor;
    bool 				m_BlockingMode;   
    bool 				m_ShowFrames;

    CameraObserver              	m_DeviceDiscoveryCallbacks;
    QSharedPointer<FrameObserver>	m_StreamCallbacks;
	
signals:
    // The camera list changed signal that passes the new camera and the its state directly
    void OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &);
    // Event will be called when a frame is processed by the internal thread and ready to show
	void OnCameraFrameReady_Signal(const QImage &image, const unsigned long long &frameId);
	// Event will be called when a frame ID is processed by the internal thread and ready to show
	void OnCameraFrameID_Signal(const unsigned long long &frameId);
	// Event will be called when the event data are processed by the internal thread and ready to show
	void OnCameraEventReady_Signal(const QString &eventText);
    // Event will be called when the any other data arrieved
	void OnCameraRegisterValueReady_Signal(unsigned long long value);
    // Event will be called on error
	void OnCameraError_Signal(const QString &text);
    // Event will be called on message
	void OnCameraMessage_Signal(const QString &text);
    // Event will be called when the a frame is recorded
	void OnCameraRecordFrame_Signal(const unsigned long long &, const unsigned long long &);
    // Event will be called when the a frame is displayed
	void OnCameraDisplayFrame_Signal(const unsigned long long &);
	
	void OnCameraPixelformat_Signal(const QString &);
	void OnCameraFramesize_Signal(const QString &);
    
private slots:
    // The event handler to set or remove devices 
	void OnCameraListChanged(const int &, unsigned int, unsigned long long, const QString &);
    // The event handler to show the processed frame
	void OnFrameReady(const QImage &image, const unsigned long long &frameId);
	// The event handler to show the processed frame ID
	void OnFrameID(const unsigned long long &frameId);
	// Event will be called when the a frame is recorded
    void OnRecordFrame(const unsigned long long &frameID, const unsigned long long &framesInQueue);
	// Event will be called when the a frame is displayed
    void OnDisplayFrame(const unsigned long long &frameID);
    // Event will be called when for text notification
    void OnMessage(const QString &msg);
    // Event will be called when for text notification
    void OnError(const QString &msg);
};

}}} // namespace AVT::Tools::Examples

#endif

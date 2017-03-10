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


#define MAX_VIEWER_USER_BUFFER_COUNT	50

class GenCPRequestPool;

enum FileType
{
    FileType_Uncompressed   = 0x00,
    FileType_ZIP            = 0x01
};
    

namespace AVT {
namespace Tools {
namespace Examples {

typedef struct _USER_BUFFER
{
    uint8_t         *pBuffer;
    size_t  		nBufferlength;
    uint8_t         *pBufferOffset;
    size_t          iSize;
    uint32_t        iReceivedLength;
    uint32_t        iReceivedLengthCounter;
    uint32_t        iSendLength;
    int             nBufferState;
    int             nProtocolState;
    int             nHandle;
    void            *pPrivateData;
} USER_BUFFER, *PUSER_BUFFER, **PPUSER_BUFFER;

class Camera : public QObject
{
    Q_OBJECT

public:
    Camera();
    virtual ~Camera();

	int OpenDevice(std::string &deviceName, bool blockingMode);
    int CloseDevice();

    int DeviceDiscoveryStart();
    int DeviceDiscoveryStop();
    int SIStartChannel(uint32_t pixelformat, uint32_t payloadsize, uint32_t width, uint32_t height, uint32_t bytesPerLine, void *pPrivateData);
    int SIStopChannel();
    
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
	int SetExposure(uint32_t exposure);
	int ReadAutoExposure(bool &autoexposure);
    int SetAutoExposure(bool autoexposure);
	
    int SendAcquisitionStart();
    int SendAcquisitionStop();

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
	unsigned int GetIncompletedFramesCount();

    // Recording
    void SetRecording(bool start);
    void DisplayStepBack();
    void DisplayStepForw();
    void DeleteRecording();

private:
    bool                                        m_bSIRunning;
    std::string 								m_DeviceName;
	int 										m_nFileDescriptor;
    bool m_BlockingMode;
    
    CameraObserver	                            m_DmaDeviceDiscoveryCallbacks;
    FrameObserver                       		m_DmaSICallbacks;
	
    uint64_t    		                        m_SIUserBufferHandles[MAX_VIEWER_USER_BUFFER_COUNT];
    //frame_t				 IO_METHOD_USERPTR                       m_SIUserFrame[MAX_VIEWER_USER_BUFFER_COUNT];
    uint32_t                                    m_UsedBufferCount;

    static uint16_t                             s_DCICmdRequestIDCounter;

	std::vector<PUSER_BUFFER>					m_UserBufferContainerList;
	// Tools
    
signals:
    // The camera list changed signal that passes the new camera and the its state directly
    void OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &);
    // Event will be called when a frame is processed by the internal thread and ready to show
	void OnCameraFrameReady_Signal(const QImage &image, const unsigned long long &frameId);
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
	void OnCameraDisplayFrame_Signal(const unsigned long long &, const unsigned long &, const unsigned long &, const unsigned long &);
	
	void OnCameraPixelformat_Signal(const QString &);
	void OnCameraFramesize_Signal(const QString &);
    
private slots:
    // The event handler to set or remove devices 
	void OnCameraListChanged(const int &, unsigned int, unsigned long long, const QString &);
    // The event handler to show the processed frame
	void OnFrameReady(const QImage &image, const unsigned long long &frameId);
	// The event handler to return the frame
	void OnFrameDone(const unsigned long long);
	// Event will be called when the a frame is recorded
    void OnRecordFrame(const unsigned long long &frameID, const unsigned long long &framesInQueue);
	// Event will be called when the a frame is displayed
    void OnDisplayFrame(const unsigned long long &frameID, const unsigned long &width, const unsigned long &height, const unsigned long &pixelformat);
    // Event will be called when for text notification
    void OnMessage(const QString &msg);
	
};

}}} // namespace AVT::Tools::Examples

#endif

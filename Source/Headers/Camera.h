/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Camera.h

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

#ifndef CAMERA_H
#define CAMERA_H

#include "FrameObserver.h"
#include "CameraObserver.h"
#include "AutoReader.h"

#include <QObject>
#include <QMutex>
#include <vector>


enum IO_METHOD_TYPE
{
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

class Camera : public QObject
{
    Q_OBJECT

public:
    Camera();
    virtual ~Camera();

    // This function performs opening of the device by the given name
    //
    // Parameters:
    // [in] (std::string &) deviceName - name of the device to open
    // [in] (bool) blockingMode - state of the blocking mode
    // [in] (IO_METHOD_TYPE) ioMethodType - type of the input output method
    // [in] (bool) v4l2TryFmt - state of the Try Fmt
    //
    // Returns:
    // (int) - result of the opening
    int OpenDevice(std::string &deviceName, bool blockingMode, IO_METHOD_TYPE ioMethodType,
                   bool v4l2TryFmt);
    // This function simply closes the opened device
    //
    // Returns:
    // (int) - result of the closing
    int CloseDevice();
    // This function starts discover of the cameras
    //
    // Returns:
    // (int) - result of the discovery start
    int DeviceDiscoveryStart();
    // This function stops discover of the cameras
    //
    // Returns:
    // (int) - result of the discovery stop
    int DeviceDiscoveryStop();

    // This function starts streaming from the camera
    //
    // Parameters:
    // [in] (uint32_t) pixelFormat - pixel format id
    // [in] (uint32_t) payloadSize - payload size
    // [in] (uint32_t) width - width of the image
    // [in] (uint32_t) height - height of the image
    // [in] (uint32_t) bytesPerLine - number of bytes per line
    // [in] (void *) pPrivateData
    // [in] (uint32_t) enableLogging - state of the logging in frame observer
    //
    // Returns:
    // (int) - result of starting streaming
    int StartStreamChannel(uint32_t pixelFormat, uint32_t payloadSize, uint32_t width, uint32_t height,
                           uint32_t bytesPerLine, void *pPrivateData,
                           uint32_t enableLogging);
    // This function stops streaming from the camera
    //
    // Returns:
    // (int) - result of the stopping streaming
    int StopStreamChannel();

    // This function reads payload size
    //
    // Parameters:
    // [in] (uint32_t &) payloadSize - payload size
    //
    // Returns:
    // (int) - result of the reading
    int ReadPayloadSize(uint32_t &payloadSize);
    // This function reads frame size
    //
    // Parameters:
    // [in] (uint32_t &) width - image width
    // [in] (uint32_t &) height - image height
    //
    // Returns:
    // (int) - result of the reading
    int ReadFrameSize(uint32_t &width, uint32_t &height);
    // This function sets frame size
    //
    // Parameters:
    // [in] (uint32_t) width - image width
    // [in] (uint32_t) height - image height
    //
    // Returns:
    // (int) - result of the setting
    int SetFrameSize(uint32_t width, uint32_t height);
    // This function read camera image width
    //
    // Parameters:
    // [in] (uint32_t &) width - image width
    //
    // Returns:
    // (int) - result of the reading
    int ReadWidth(uint32_t &width);
    // This function sets camera image width
    //
    // Parameters:
    // [in] (uint32_t) width - image width
    //
    // Returns:
    // (int) - result of the setting
    int SetWidth(uint32_t width);
    // This function read camera image height
    //
    // Parameters:
    // [in] (uint32_t &) height - image height
    //
    // Returns:
    // (int) - result of the reading
    int ReadHeight(uint32_t &height);
    // This function set camera image height
    //
    // Parameters:
    // [in] (uint32_t) height - image height
    //
    // Returns:
    // (int) - result of the setting
    int SetHeight(uint32_t height);
    // This function read camera pixel format
    //
    // Parameters:
    // [in] (uint32_t &) pixelFormat - image pixelFormat
    // [in] (uint32_t &) bytesPerLine - bytesPerLine
    // [in] (QString &) pfText - pixel format text
    //
    // Returns:
    // (int) - result of the reading
    int ReadPixelFormat(uint32_t &pixelFormat, uint32_t &bytesPerLine, QString &pfText);
    // This function set camera pixel format
    //
    // Parameters:
    // [in] (uint32_t) pixelFormat - image pixelFormat
    // [in] (QString) pfText - pixel format text
    //
    // Returns:
    // (int) - result of the setting
    int SetPixelFormat(uint32_t pixelFormat, QString pfText);
    // This function reads all pixel formats and emits signal with pixel format list
    //
    // Returns:
    // (int) - result of reading
    int ReadFormats();
    // This function read camera gain
    //
    // Parameters:
    // [in] (uint32_t &) gain
    //
    // Returns:
    // (int) - result of the reading
    int ReadGain(int32_t &gain);
    // This function set camera gain
    //
    // Parameters:
    // [in] (uint32_t) gain
    //
    // Returns:
    // (int) - result of the setting
    int SetGain(int32_t gain);
    // This function read camera min max gain
    //
    // Parameters:
    // [in] (uint32_t &) min
    // [in] (uint32_t &) max
    //
    // Returns:
    // (int) - result of the reading
    int ReadMinMaxGain(int32_t &min, int32_t &max);
    // This function reads auto gain state
    //
    // Parameters:
    // [in] (bool &) autogain - state of the auto gain
    //
    // Returns:
    // (int) - result of the reading
    int ReadAutoGain(bool &autogain);
    // This function sets auto gain state
    //
    // Parameters:
    // [in] (bool &) autogain - state of the auto gain
    //
    // Returns:
    // (int) - result of the setting
    int SetAutoGain(bool autogain);

    // This function reads exposure
    //
    // Parameters:
    // [in] (int32_t &) exposure
    //
    // Returns:
    // (int) - result of reading
    int ReadExposure(int32_t &exposure);
    // This function reads exposure absolute
    //
    // Parameters:
    // [in] (int32_t &) exposure
    //
    // Returns:
    // (int) - result of reading
    int ReadExposureAbs(int32_t &exposure);
    // This function sets exposure
    //
    // Parameters:
    // [in] (int32_t) exposure
    //
    // Returns:
    // (int) - result of setting
    int SetExposure(int32_t exposure);
    // This function sets exposure absolute
    //
    // Parameters:
    // [in] (int32_t) exposure
    //
    // Returns:
    // (int) - result of setting
    int SetExposureAbs(int32_t exposure);
    // This function reads min and max of the exposure
    //
    // Parameters:
    // [in] (int32_t &) min - minimum exposure
    // [in] (int32_t &) max - maximum exposure
    //
    // Returns:
    // (int) - result of reading
    int ReadMinMaxExposure(int32_t &min, int32_t &max);
    // This function reads min and max of the exposure absolute
    //
    // Parameters:
    // [in] (int32_t &) min - minimum exposure
    // [in] (int32_t &) max - maximum exposure
    //
    // Returns:
    // (int) - result of reading
    int ReadMinMaxExposureAbs(int32_t &min, int32_t &max);
    // This function reads state of the auto exposure
    //
    // Parameters:
    // [in] (bool &) autoexposure
    //
    // Returns:
    // (int) - result of reading
    int ReadAutoExposure(bool &autoexposure);
    // This function turns on auto exposure
    //
    // Parameters:
    // [in] (bool) autoexposure
    //
    // Returns:
    // (int) - result of setting
    int SetAutoExposure(bool autoexposure);

    // This function reads gamma
    //
    // Parameters:
    // [in] (int32_t &) value - read value
    //
    // Returns:
    // (int) - result of reading
    int ReadGamma(int32_t &value);
    // This function sets gamma
    //
    // Parameters:
    // [in] (int32_t) value - new value to be set
    //
    // Returns:
    // (int) - result of setting
    int SetGamma(int32_t value);
    // This function reads min/max gamma
    //
    // Parameters:
    // [in] (int32_t &) min - minimum gamma
    // [in] (int32_t &) max - maximum gamma
    //
    // Returns:
    // (int) - result of reading
    int ReadMinMaxGamma(int32_t &min, int32_t &max);

    // This function reads reverseX value
    //
    // Parameters:
    // [in] (int32_t &) value - reverseX value
    //
    // Returns:
    // (int) - result of reading
    int ReadReverseX(int32_t &value);
    // This function sets reverseX value
    //
    // Parameters:
    // [in] (int32_t) value - new reverseX value
    //
    // Returns:
    // (int) - result of setting
    int SetReverseX(int32_t value);
    // This function reads reverseY value
    //
    // Parameters:
    // [in] (int32_t &) value - reverseY value
    //
    // Returns:
    // (int) - result of reading
    int ReadReverseY(int32_t &value);
    // This function sets reverseY value
    //
    // Parameters:
    // [in] (int32_t) value - new reverseY value
    //
    // Returns:
    // (int) - result of setting
    int SetReverseY(int32_t value);

    // This function reads brightness/blacklevel
    //
    // Parameters:
    // [in] (int32_t &) value - blacklevel value
    //
    // Returns:
    // (int) - result of reading
    int ReadBrightness(int32_t &value);
    // This function sets brightness/blacklevel
    //
    // Parameters:
    // [in] (int32_t &) value - new blacklevel value
    //
    // Returns:
    // (int) - result of setting
    int SetBrightness(int32_t value);
    // This function reads minimum and maximum brightness/blacklevel
    //
    // Parameters:
    // [in] (int32_t &) min - minimum value
    // [in] (int32_t &) max - maximum value
    //
    // Returns:
    // (int) - result of reading
    int ReadMinMaxBrightness(int32_t &min, int32_t &max);

    // This function sets state of the continuous white balance
    //
    // Parameters:
    // [in] (bool) flag - new state of the continuous white balance
    //
    // Returns:
    // (int) - result of setting
    int SetContinousWhiteBalance(bool flag);
    // This function reads auto white balance support by camera
    //
    // Returns:
    // (bool) - support state
    bool IsAutoWhiteBalanceSupported();

    int ReadAutoWhiteBalance(bool &flag);
    int ReadRedBalance(int32_t &value);
    int SetRedBalance(int32_t value);
    int ReadBlueBalance(int32_t &value);
    int SetBlueBalance(int32_t value);
    int ReadFrameRate(uint32_t &numerator, uint32_t &denominator, uint32_t width, uint32_t height, uint32_t pixelFormat);
    int SetFrameRate(uint32_t numerator, uint32_t denominator);

    int SetExtControl(uint32_t value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);
    int ReadExtControl(uint32_t &value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);

    int SetExtControl(int32_t value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);
    int ReadExtControl(int32_t &value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);

    int SetExtControl(uint64_t value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);
    int ReadExtControl(uint64_t &value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);

    int SetExtControl(int64_t value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);
    int ReadExtControl(int64_t &value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);

    int ReadMinMax(uint32_t &min, uint32_t &max, uint32_t controlID, const char *functionName, const char* controlName);
    int ReadMinMax(int32_t &min, int32_t &max, uint32_t controlID, const char *functionName, const char* controlName);

    int ReadStep(int32_t &step, uint32_t controlID, const char *functionName, const char* controlName);

    int ReadRegister(uint16_t nRegAddr, void* pBuffer, uint32_t nBufferSize, bool bConvertEndianess);
    int WriteRegister(uint16_t nRegAddr, void* pBuffer, uint32_t nBufferSize, bool bConvertEndianess);

    int EnumAllControlNewStyle();

    int ReadCropCapabilities(uint32_t &boundsx, uint32_t &boundsy, uint32_t &boundsw, uint32_t &boundsh,
                             uint32_t &defrectx, uint32_t &defrecty, uint32_t &defrectw, uint32_t &defrecth,
                             uint32_t &aspectnum, uint32_t &aspectdenum);
    int ReadCrop(uint32_t &xOffset, uint32_t &yOffset, uint32_t &width, uint32_t &height);
    int SetCrop(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height);

    int StartStreaming();
    int StopStreaming();

    int CreateUserBuffer(uint32_t bufferCount, uint32_t bufferSize);
    int QueueAllUserBuffer();
    int QueueSingleUserBuffer(const int index);
    int DeleteUserBuffer();

    // Info
    int GetCameraDriverName(std::string &strText);
    int GetCameraDeviceName(std::string &strText);
    int GetCameraBusInfo(std::string &strText);
    int GetCameraDriverVersion(std::string &strText);
    int GetCameraReadCapability(bool &flag);
    int GetCameraCapabilities(std::string &strText);

    // Statistics
    bool getDriverStreamStat(uint64_t &FramesCount, uint64_t &PacketCRCError, uint64_t &FramesUnderrun, uint64_t &FramesIncomplete, double &CurrentFrameRate);
    unsigned int GetReceivedFramesCount();
    unsigned int GetRenderedFramesCount();
    unsigned int GetDroppedFramesCount();

    // Exposure Active
    int ReadExposureActiveLineMode(bool &state);
    int ReadExposureActiveLineSelector(int32_t &value, int32_t &min, int32_t &max, int32_t &step);
    int ReadExposureActiveInvert(bool &state);

    int SetExposureActiveLineMode(bool state);
    int SetExposureActiveLineSelector(int32_t value);
    int SetExposureActiveInvert(bool state);

    // Misc
    void SwitchFrameTransfer2GUI(bool showFrames);

    std::string getAvtDeviceFirmwareVersion();
    std::string getAvtDeviceTemperature();
    std::string getAvtDeviceSerialNumber();

    static std::string ConvertErrno2String(int errnumber);
    static std::string ConvertPixelFormat2String(int pixelFormat);
    static std::string ConvertPixelFormat2EnumString(int pixelFormat);
    static std::string ConvertControlID2String(uint32_t controlID);


public slots:
    void PassGainValue();
    void PassExposureValue();
    void PassWhiteBalanceValue();

    void SetEnumerationControlValueIntList(int32_t id, int64_t val);
    void SetEnumerationControlValueList(int32_t id, const char* str); //Values passed to controls which accept strings
    void SetEnumerationControlValue(int32_t id, int32_t val); //Values passed to controls which accept 32 bit ingegers
    void SetEnumerationControlValue(int32_t id, int64_t val); //Values passed to controls which accept 64 bit integers
    void SetEnumerationControlValue(int32_t id, bool val); //Values passed to controls which accept booleans
    void SetEnumerationControlValue(int32_t id); //Values passed to controls which performs some actions on button click
    void SetSliderEnumerationControlValue(int32_t id, int32_t val); //Value passed to controls from the slider
    void SetSliderEnumerationControlValue(int32_t id, int64_t val); //Value passed to controls from the slider 64 bit

private:
    std::string         m_DeviceName;
    int                 m_nFileDescriptor;
    bool                m_BlockingMode;
    bool                m_ShowFrames;
    bool                m_UseV4L2TryFmt;
    bool                m_Recording;
    bool                m_IsAvtCamera;
    QMutex              m_ReadExtControlMutex;

    AutoReader          *m_pAutoExposureReader;
    AutoReader          *m_pAutoGainReader;
    AutoReader          *m_pAutoWhiteBalanceReader;

    CameraObserver                  m_CameraObserver;
    QSharedPointer<FrameObserver>   m_pFrameObserver;

    std::vector<uint8_t>        m_CsvData;

    size_t fsize(const char *filename);
    void reverseBytes(void* start, int size);


signals:
    // The camera list changed signal that passes the new camera and the its state directly
    void OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &, const QString &);
    // Event will be called when a frame is processed by the internal thread and ready to show
    void OnCameraFrameReady_Signal(const QImage &image, const unsigned long long &frameId);
    // Event will be called when a frame ID is processed by the internal thread and ready to show
    void OnCameraFrameID_Signal(const unsigned long long &frameId);
    // Event will be called when the a frame is recorded
    void OnCameraRecordFrame_Signal(const QSharedPointer<MyFrame>&);
    // Event will be called when the a frame is displayed
    void OnCameraDisplayFrame_Signal(const unsigned long long &);
    void OnCameraPixelFormat_Signal(const QString &);
    void OnCameraFrameSize_Signal(const QString &);

    void PassAutoExposureValue(int32_t value);
    void PassAutoGainValue(int32_t value);
    void PassAutoWhiteBalanceValue(int32_t value);

    void SendIntDataToEnumerationWidget(int32_t id, int32_t min, int32_t max, int32_t value, QString name, QString unit, bool bIsReadOnly);
    void SentInt64DataToEnumerationWidget(int32_t id, int64_t min, int64_t max, int64_t value, QString name, QString unit, bool bIsReadOnly);
    void SendBoolDataToEnumerationWidget(int32_t id, bool value, QString name, QString unit, bool bIsReadOnly);
    void SendButtonDataToEnumerationWidget(int32_t id, QString name, QString unit, bool bIsReadOnly);

    void SendListDataToEnumerationWidget(int32_t id, int32_t value, QList<QString> list, QString name, QString unit, bool bIsReadOnly);
    void SendListIntDataToEnumerationWidget(int32_t id, int32_t value, QList<int64_t> list, QString name, QString unit, bool bIsReadOnly);

    void SendSignalToUpdateWidgets();

private slots:
    // The event handler to set or remove devices
    void OnCameraListChanged(const int &, unsigned int, unsigned long long, const QString &, const QString &);
    // The event handler to show the processed frame
    void OnFrameReady(const QImage &image, const unsigned long long &frameId);
    // The event handler to show the processed frame ID
    void OnFrameID(const unsigned long long &frameId);
    // Event will be called when the a frame is displayed
    void OnDisplayFrame(const unsigned long long &frameID);
};

#endif // CAMERA_H

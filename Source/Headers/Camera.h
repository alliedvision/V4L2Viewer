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
#include <QDebug>
#include <vector>


enum IO_METHOD_TYPE
{
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

class Camera : public QObject
{
    Q_OBJECT

public:
    Camera();
    virtual ~Camera();

    int OpenDevice(std::string &deviceName, bool blockingMode, IO_METHOD_TYPE ioMethodType,
                   bool v4l2TryFmt);
    int CloseDevice();

    int DeviceDiscoveryStart();
    int DeviceDiscoveryStop();
    int StartStreamChannel(uint32_t pixelFormat, uint32_t payloadSize, uint32_t width, uint32_t height,
                           uint32_t bytesPerLine, void *pPrivateData,
                           uint32_t enableLogging);
    int StopStreamChannel();

    int ReadPayloadSize(uint32_t &payloadSize);
    int ReadFrameSize(uint32_t &width, uint32_t &height);
    int SetFrameSize(uint32_t width, uint32_t height);
    int ReadWidth(uint32_t &width);
    int SetWidth(uint32_t width);
    int ReadHeight(uint32_t &height);
    int SetHeight(uint32_t height);
    int ReadPixelFormat(uint32_t &pixelFormat, uint32_t &bytesPerLine, QString &pfText);
    int SetPixelFormat(uint32_t pixelFormat, QString pfText);
    int ReadFormats();

    int ReadGain(int32_t &gain);
    int SetGain(int32_t gain);
    int ReadMinMaxGain(int32_t &min, int32_t &max);
    int ReadAutoGain(bool &autogain);
    int SetAutoGain(bool autogain);

    int ReadExposure(int32_t &exposure);
    int ReadExposureAbs(int32_t &exposure);
    int SetExposure(int32_t exposure);
    int SetExposureAbs(int32_t exposure);
    int ReadMinMaxExposure(int32_t &min, int32_t &max);
    int ReadMinMaxExposureAbs(int32_t &min, int32_t &max);
    int ReadAutoExposure(bool &autoexposure);
    int SetAutoExposure(bool autoexposure);

    int ReadGamma(int32_t &value);
    int SetGamma(int32_t value);
    int ReadMinMaxGamma(int32_t &min, int32_t &max);

    int ReadReverseX(int32_t &value);
    int SetReverseX(int32_t value);
    int ReadReverseY(int32_t &value);
    int SetReverseY(int32_t value);

    int ReadBrightness(int32_t &value);
    int SetBrightness(int32_t value);
    int ReadMinMaxBrightness(int32_t &min, int32_t &max);

    int ReadContrast(int32_t &value);
    int SetContrast(int32_t value);
    int ReadSaturation(int32_t &value);
    int SetSaturation(int32_t value);
    int ReadHue(int32_t &value);
    int SetHue(int32_t value);
    int SetContinousWhiteBalance(bool flag);
    bool IsAutoWhiteBalanceSupported();
    bool IsWhiteBalanceOnceSupported();
    int DoWhiteBalanceOnce();
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

    void SendIntDataToEnumerationWidget(int32_t id, int32_t step, int32_t min, int32_t max, int32_t value, QString name);
    void SentInt64DataToEnumerationWidget(int32_t id, int64_t step, int64_t min, int64_t max, int64_t value, QString name);
    void SendBoolDataToEnumerationWidget(int32_t id, bool value, QString name);
    void SendButtonDataToEnumerationWidget(int32_t id, QString name);

    void SendListDataToEnumerationWidget(int32_t id, QList<QString> list, QString name);
    void SendListIntDataToEnumerationWidget(int32_t id, QList<int64_t> list, QString name);

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

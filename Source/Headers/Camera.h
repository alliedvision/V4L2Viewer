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
    int ReadGain(int64_t &gain);
    // This function set camera gain
    //
    // Parameters:
    // [in] (uint32_t) gain
    //
    // Returns:
    // (int) - result of the setting
    int SetGain(int64_t gain);
    // This function read camera min max gain
    //
    // Parameters:
    // [in] (uint32_t &) min
    // [in] (uint32_t &) max
    //
    // Returns:
    // (int) - result of the reading
    int ReadMinMaxGain(int64_t &min, int64_t &max);
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
    int ReadMinMaxExposureAbs(int64_t &min, int64_t &max);
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
    int SetAutoWhiteBalance(bool flag);
    // This function reads auto white balance support by camera
    //
    // Returns:
    // (bool) - support state
    bool IsAutoWhiteBalanceSupported();

    // This function reads auto white balance state
    //
    // Parameters:
    // [in] (bool &) flag - state of the auto white balance
    //
    // Returns:
    // (int) - result of the reading
    int ReadAutoWhiteBalance(bool &flag);

    // This function reads two values, numerator and denominator
    // for the frame rate
    //
    // Parameters:
    // [in] (uint32_t &) numerator - numerator to be read
    // [in] (uint32_t &) denominator - denominator to be read
    // [in] (uint32_t) width - given width
    // [in] (uint32_t) height - given height
    // [in] (uint32_t) pixelFormat - given pixel format
    //
    // Returns:
    // (int) - result of reading
    int ReadFrameRate(uint32_t &numerator, uint32_t &denominator, uint32_t width, uint32_t height, uint32_t pixelFormat);
    // This function sets numerator and denominator
    // for the frame rate
    //
    // Parameters:
    // [in] (uint32_t) numerator - numerator to be set
    // [in] (uint32_t) denominator - denominator to be set
    //
    // Returns:
    // (int) - result of setting
    int SetFrameRate(uint32_t numerator, uint32_t denominator);

    // This function is used by all of the controls in the application,
    // it sets control's value using ext approach
    //
    // Parameters:
    // [in] (uint32_t) value - new value
    // [in] (uint32_t) controlID - controlID
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (used in logger)
    // [in] (uint32_t) controlClass - class of the control
    //
    // Returns:
    // (int) - result of the setting
    int SetExtControl(uint32_t value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);
    // This function is used by all of the controls in the application,
    // it reads control's value using ext approach
    //
    // Parameters:
    // [in] (uint32_t &) value - new value
    // [in] (uint32_t) controlID - controlID
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (used in logger)
    // [in] (uint32_t) controlClass - class of the control
    //
    // Returns:
    // (int) - result of the reading
    int ReadExtControl(uint32_t &value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);

    // This function is used by all of the controls in the application,
    // it sets control's value using ext approach
    //
    // Parameters:
    // [in] (int32_t) value - new value
    // [in] (int32_t) controlID - controlID
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (used in logger)
    // [in] (uint32_t) controlClass - class of the control
    //
    // Returns:
    // (int) - result of the setting
    int SetExtControl(int32_t value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);
    // This function is used by all of the controls in the application,
    // it reads control's value using ext approach
    //
    // Parameters:
    // [in] (int32_t &) value - new value
    // [in] (int32_t) controlID - controlID
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (used in logger)
    // [in] (uint32_t) controlClass - class of the control
    //
    // Returns:
    // (int) - result of the reading
    int ReadExtControl(int32_t &value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);

    // This function is used by all of the controls in the application,
    // it sets control's value using ext approach
    //
    // Parameters:
    // [in] (uint64_t) value - new value
    // [in] (uint32_t) controlID - controlID
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (used in logger)
    // [in] (uint32_t) controlClass - class of the control
    //
    // Returns:
    // (int) - result of the setting
    int SetExtControl(uint64_t value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);
    // This function is used by all of the controls in the application,
    // it reads control's value using ext approach
    //
    // Parameters:
    // [in] (uint64_t &) value - new value
    // [in] (uint32_t) controlID - controlID
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (used in logger)
    // [in] (uint32_t) controlClass - class of the control
    //
    // Returns:
    // (int) - result of the reading
    int ReadExtControl(uint64_t &value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);

    // This function is used by all of the controls in the application,
    // it sets control's value using ext approach
    //
    // Parameters:
    // [in] (int64_t) value - new value
    // [in] (int32_t) controlID - controlID
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (used in logger)
    // [in] (uint32_t) controlClass - class of the control
    //
    // Returns:
    // (int) - result of the setting
    int SetExtControl(int64_t value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);
    // This function is used by all of the controls in the application,
    // it reads control's value using ext approach
    //
    // Parameters:
    // [in] (int64_t &) value - new value
    // [in] (int32_t) controlID - controlID
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (used in logger)
    // [in] (uint32_t) controlClass - class of the control
    //
    // Returns:
    // (int) - result of the reading
    int ReadExtControl(int64_t &value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass);

    // This function reads min and max of the given control
    //
    // Parameters:
    // [in] (uint32_t &) min - minimum value
    // [in] (uint32_t &) max - maximum value
    // [in] (uint32_t) controlID - id of the control
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (uised in logger)
    //
    // Returns:
    // (int) - result of reading
    int ReadMinMax(uint32_t &min, uint32_t &max, uint32_t controlID, const char *functionName, const char* controlName);
    // This function reads min and max of the given control
    //
    // Parameters:
    // [in] (int32_t &) min - minimum value
    // [in] (int32_t &) max - maximum value
    // [in] (uint32_t) controlID - id of the control
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (uised in logger)
    //
    // Returns:
    // (int) - result of reading
    int ReadMinMax(int32_t &min, int32_t &max, uint32_t controlID, const char *functionName, const char* controlName);
    // This function reads min and max of the given control
    //
    // Parameters:
    // [in] (int64_t &) min - minimum value
    // [in] (int64_t &) max - maximum value
    // [in] (uint32_t) controlID - id of the control
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (uised in logger)
    //
    // Returns:
    // (int) - result of reading
    int ReadMinMax(int64_t &min, int64_t &max, uint32_t controlID, const char *functionName, const char* controlName);

    // This function reads step of the given control
    //
    // Parameters:
    // [in] (int32_t &) step - step value
    // [in] (uint32_t) controlID - id of the control
    // [in] (const char *) functionName - name of the function (used in logger)
    // [in] (const char *) controlName - name of the control (uised in logger)
    //
    // Returns:
    // (int) - result of reading
    int ReadStep(int32_t &step, uint32_t controlID, const char *functionName, const char* controlName);

    // This function reads register
    //
    // Parameters:
    // [in] (uint16_t) nRegAddr - register address
    // [in] (void *) pBuffer - buffer
    // [in] (uint32_t) nBufferSize - size of the buffer
    // [in] (bool) bConvertEndianess - state of the endianess converting
    //
    // Returns:
    // (int) - result of reading
    int ReadRegister(uint16_t nRegAddr, void* pBuffer, uint32_t nBufferSize, bool bConvertEndianess);
    // This function writes register
    //
    // Parameters:
    // [in] (uint16_t) nRegAddr - register address
    // [in] (void *) pBuffer - buffer
    // [in] (uint32_t) nBufferSize - size of the buffer
    // [in] (bool) bConvertEndianess - state of the endianess converting
    //
    // Returns:
    // (int) - result of writing
    int WriteRegister(uint16_t nRegAddr, void* pBuffer, uint32_t nBufferSize, bool bConvertEndianess);

    // This function enumerates all of the controls
    // and emits found controls to the GUI class to create widgets.
    //
    // Returns:
    // (int) - result of the enumeration
    int EnumAllControlNewStyle();

    // This function reads crop
    //
    // Parameters:
    // [in] (uint32_t &) xOffset
    // [in] (uint32_t &) yOffset
    // [in] (uint32_t &) width
    // [in] (uint32_t &) height
    //
    // Returns
    // (int) - result of the reading
    int ReadCrop(uint32_t &xOffset, uint32_t &yOffset, uint32_t &width, uint32_t &height);
    // This function sets crop
    //
    // Parameters:
    // [in] (uint32_t) xOffset
    // [in] (uint32_t) yOffset
    // [in] (uint32_t) width
    // [in] (uint32_t) height
    //
    // Returns
    // (int) - result of the setting
    int SetCrop(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height);

    // This function starts streaming from the camera
    //
    // Returns:
    // (int) - result of stream starting
    int StartStreaming();
    // This function stops streaming from the camera
    //
    // Returns:
    // (int) - result of stream stopping
    int StopStreaming();

    // This function creates user buffer
    //
    // Parameters:
    // [in] (uint32_t) bufferCount
    // [in] (uint32_t) bufferSize
    //
    // Returns:
    // (int) - result of buffer creation
    int CreateUserBuffer(uint32_t bufferCount, uint32_t bufferSize);
    // This function queues all of the user's buffer
    //
    // Returns:
    // (int) - result of queuing
    int QueueAllUserBuffer();
    // This function queues single buffer by given index
    //
    // Parameters:
    // [in] (const int) index - given index
    //
    // Returns:
    // (int) - result of queuing single buffer
    int QueueSingleUserBuffer(const int index);
    // This function deletes user buffer
    //
    // Returns:
    // (int) - result of deleting
    int DeleteUserBuffer();

    // This function returns camera driver name
    //
    // Parameters:
    // [in] (std::string &) strText - camera driver name
    //
    // Returns:
    // (int) - result of the operation
    int GetCameraDriverName(std::string &strText);
    // This function returns camera device name
    //
    // Parameters:
    // [in] (std::string &) strText - camera device name
    //
    // Returns:
    // (int) - result of the operation
    int GetCameraDeviceName(std::string &strText);
    // This function returns camera bus info
    //
    // Parameters:
    // [in] (std::string &) strText - camera bus info
    //
    // Returns:
    // (int) - result of the operation
    int GetCameraBusInfo(std::string &strText);
    // This function returns camera driver version
    //
    // Parameters:
    // [in] (std::string &) strText - camera driver version
    //
    // Returns:
    // (int) - result of the operation
    int GetCameraDriverVersion(std::string &strText);
    // This function returns camera capability state
    //
    // Parameters:
    // [in] (bool &) flag - camera capability state
    //
    // Returns:
    // (int) - result of the operation
    int GetCameraReadCapability(bool &flag);
    // This function returns camera capabilities
    //
    // Parameters:
    // [in] (std::string &) strText - camera capabilities
    //
    // Returns:
    // (int) - result of the operation
    int GetCameraCapabilities(std::string &strText);

    // This function returns driver stream statistics
    //
    // Parameters:
    // [in] (uint64_t &) FramesCount
    // [in] (uint64_t &) PacketCRCError
    // [in] (uint64_t &) FramesUnderrun
    // [in] (uint64_t &) FramesIncomplete - number of incomplete frames
    // [in] (double &) CurrentFrameRate
    //
    // Returns:
    // (bool) - result of the gathering statistics
    bool getDriverStreamStat(uint64_t &FramesCount, uint64_t &PacketCRCError, uint64_t &FramesUnderrun, uint64_t &FramesIncomplete, double &CurrentFrameRate);
    // This function returns received frames count
    //
    // Returns:
    // (unsigned int) - number of received frames
    unsigned int GetReceivedFramesCount();
    // This function returns rendered frames count
    //
    // Returns:
    // (unsigned int) - number of rendered frames
    unsigned int GetRenderedFramesCount();

    // This function switches frame transfer to gui
    //
    // Parameters:
    // [in] (bool) showFrames - state of frames visibility
    void SwitchFrameTransfer2GUI(bool showFrames);

    // This function returns AVT Device firmware version
    //
    // Returns:
    // (std::string) - firmware version
    std::string getAvtDeviceFirmwareVersion();
    // This function returns AVT Device Temperature
    //
    // Returns:
    // (std::string) - device temperature
    std::string getAvtDeviceTemperature();
    // This function returns AVT Device Serial Number
    //
    // Returns:
    // (std::string) - device serial number
    std::string getAvtDeviceSerialNumber();

    // This function converts errors to string and returns it
    //
    // Parameters:
    // [in] (int) errnumber - error number
    //
    // Returns:
    // std::string - error
    static std::string ConvertErrno2String(int errnumber);
    // This function converts pixel format to string and returns it
    //
    // Parameters:
    // [in] (int) pixelFormat - pixel format
    //
    // Returns:
    // std::string - pixelFormat
    static std::string ConvertPixelFormat2String(int pixelFormat);
    // This function converts pixel format to enum string and returns it
    //
    // Parameters:
    // [in] (int) pixelFormat - pixel format
    //
    // Returns:
    // std::string - enum pixelFormat
    static std::string ConvertPixelFormat2EnumString(int pixelFormat);
    // This function converts control id to string and returns it
    //
    // Parameters:
    // [in] (int) controlID - control id
    //
    // Returns:
    // std::string - control id
    static std::string ConvertControlID2String(uint32_t controlID);


public slots:
    // This slot function passes gain value from a thread
    // to a gui when auto is turned on
    void PassGainValue();
    // This slot function passes exposure value from a thread
    // to a gui when auto is turned on
    void PassExposureValue();

    // This slot function sets enumeration integer list control value
    //
    // Parameters:
    // [in] (int32_t) id - id of the control
    // [in] (int64_t) val - new value of the control
    void SetEnumerationControlValueIntList(int32_t id, int64_t val);
    // This slot function sets enumeration list control value
    //
    // Parameters:
    // [in] (int32_t) id - id of the control
    // [in] (const char*) str - new string value of the control
    void SetEnumerationControlValueList(int32_t id, const char* str);
    // This slot function sets enumeration integer control value
    //
    // Parameters:
    // [in] (int32_t) id - id of the control
    // [in] (int32_t) val - new control value
    void SetEnumerationControlValue(int32_t id, int32_t val);
    // This slot function sets enumeration 64-bit integer control value
    //
    // Parameters:
    // [in] (int32_t) id - id of the control
    // [in] (int64_t) val - new control value
    void SetEnumerationControlValue(int32_t id, int64_t val);
    // This slot function sets enumeration boolean control value
    //
    // Parameters:
    // [in] (int32_t) id - id of the control
    // [in] (bool) val - new control value
    void SetEnumerationControlValue(int32_t id, bool val);
    // This slot function performs action on enuemration button control click
    //
    // Parameters:
    // [in] (int32_t) id - control id
    void SetEnumerationControlValue(int32_t id);
    // This slot function sets enumeration integer control value from the slider
    //
    // Parameters:
    // [in] (int32_t) id - id of the control
    // [in] (int32_t) val - new control value
    void SetSliderEnumerationControlValue(int32_t id, int32_t val);
    // This slot function sets enumeration 64-bit integer control value from the slider
    //
    // Parameters:
    // [in] (int32_t) id - id of the control
    // [in] (int64_t) val - new control value
    void SetSliderEnumerationControlValue(int32_t id, int64_t val);

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
    // Event will be called when found new pixel format
    void OnCameraPixelFormat_Signal(const QString &);
    // Event will be called on new frame size
    void OnCameraFrameSize_Signal(const QString &);

    // Event will be called on signal from thread when auto exposure is turend on
    void PassAutoExposureValue(int32_t value);
    // Event will be called on signal from thread when auto gain is turend on
    void PassAutoGainValue(int32_t value);
    // Event will be called on signal from thread when auto white balance is turend on
    void PassAutoWhiteBalanceValue(int32_t value);

    // Event will be called on enumeration control is found
    void SendIntDataToEnumerationWidget(int32_t id, int32_t min, int32_t max, int32_t value, QString name, QString unit, bool bIsReadOnly);
    // Event will be called on enumeration control is found
    void SentInt64DataToEnumerationWidget(int32_t id, int64_t min, int64_t max, int64_t value, QString name, QString unit, bool bIsReadOnly);
    // Event will be called on enumeration control is found
    void SendBoolDataToEnumerationWidget(int32_t id, bool value, QString name, QString unit, bool bIsReadOnly);
    // Event will be called on enumeration control is found
    void SendButtonDataToEnumerationWidget(int32_t id, QString name, QString unit, bool bIsReadOnly);

    // Event will be called on enumeration control is found
    void SendListDataToEnumerationWidget(int32_t id, int32_t value, QList<QString> list, QString name, QString unit, bool bIsReadOnly);
    // Event will be called on enumeration control is found
    void SendListIntDataToEnumerationWidget(int32_t id, int32_t value, QList<int64_t> list, QString name, QString unit, bool bIsReadOnly);

    // Event will be called on enumeration control change value
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

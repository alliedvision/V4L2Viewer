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


#include "Camera.h"
#include "FrameObserverMMAP.h"
#include "FrameObserverUSER.h"
#include "IOHelper.h"
#include "Logger.h"
#include "MemoryHelper.h"
#include "V4L2Helper.h"

#include <QStringList>
#include <QSysInfo>
#include <QMutexLocker>
#include <QFile>

#include <errno.h>
#include <fcntl.h>
#include <IOHelper.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <sstream>
#include <iomanip>

// Custom ioctl definitions
struct v4l2_i2c
{
    __u32       register_address;       // Register
    __u32       timeout;                // Timeout value
    const char* ptr_buffer;             // I/O buffer
    __u32       register_size;          // Register size
    __u32       num_bytes;              // Bytes to read
};

// i2c read
#define VIDIOC_R_I2C                        _IOWR('V', BASE_VIDIOC_PRIVATE + 0, struct v4l2_i2c)

// i2c write
#define VIDIOC_W_I2C                        _IOWR('V', BASE_VIDIOC_PRIVATE + 1, struct v4l2_i2c)


// Stream statistics
struct v4l2_stats_t
{
    __u64 frames_count;                 // Total number of frames received
    __u64 packet_crc_error;             // Number of packets with CRC errors
    __u64 frames_underrun;              // Number of frames dropped because of buffer underrun
    __u64 frames_incomplete;            // Number of frames that were not completed
    __u64 current_frame_count;          // Number of frames received within CurrentFrameInterval (nec. to calculate fps value)
    __u64 current_frame_interval;       // Time interval between frames in µs
};

#define VIDIOC_STREAMSTAT                   _IOR('V', BASE_VIDIOC_PRIVATE + 5, struct v4l2_stats_t)

#define V4L2_CID_PREFFERED_STRIDE               (V4L2_CID_CAMERA_CLASS_BASE+5998)

class IPixFormat
{
public:
    virtual
    ~IPixFormat() = default;

public:
    virtual
    uint32_t GetSizeImage(const v4l2_format& fmt) = 0;

    virtual
    uint32_t GetWidth(const v4l2_format& fmt) = 0;

    virtual
    uint32_t GetHeight(const v4l2_format& fmt) = 0;

    virtual
    uint32_t GetField(const v4l2_format& fmt) = 0;

    virtual
    uint32_t GetBytesPerLine(const v4l2_format& fmt) = 0;

    virtual
    uint32_t GetPixelFormat(const v4l2_format& fmt) = 0;

    virtual
    void SetSizeImage(v4l2_format& fmt, const uint32_t sizeImage) = 0;

    virtual
    void SetWidth(v4l2_format& fmt, const uint32_t width) = 0;

    virtual
    void SetHeight(v4l2_format& fmt, const uint32_t height) = 0;

    virtual
    void SetField(v4l2_format& fmt, const uint32_t field) = 0;

    virtual
    void SetBytesPerLine(v4l2_format& fmt, const uint32_t bytesPerLine) = 0;

    virtual
    void SetPixelFormat(v4l2_format& fmt, const uint32_t pixelFormat) = 0;
};

class PixFormat : public IPixFormat
{
public:
    virtual
    uint32_t GetSizeImage(const v4l2_format& fmt) override {return fmt.fmt.pix.sizeimage;}

    virtual
    uint32_t GetWidth(const v4l2_format& fmt) override {return fmt.fmt.pix.width;}

    virtual
    uint32_t GetHeight(const v4l2_format& fmt) override {return fmt.fmt.pix.height;}

    virtual
    uint32_t GetField(const v4l2_format& fmt) override {return fmt.fmt.pix.field;}

    virtual
    uint32_t GetBytesPerLine(const v4l2_format& fmt) override {return fmt.fmt.pix.bytesperline;}

    virtual
    uint32_t GetPixelFormat(const v4l2_format& fmt) override {return fmt.fmt.pix.pixelformat;}

    virtual
    void SetSizeImage(v4l2_format& fmt, const uint32_t sizeImage) override {fmt.fmt.pix.sizeimage = sizeImage;}

    virtual
    void SetWidth(v4l2_format& fmt, const uint32_t width) override {fmt.fmt.pix.width = width;}

    virtual
    void SetHeight(v4l2_format& fmt, const uint32_t height) override {fmt.fmt.pix.height = height;}

    virtual
    void SetField(v4l2_format& fmt, const uint32_t field) override {fmt.fmt.pix.field = field;}

    virtual
    void SetBytesPerLine(v4l2_format& fmt, const uint32_t bytesPerLine) override {fmt.fmt.pix.bytesperline = bytesPerLine;}

    virtual
    void SetPixelFormat(v4l2_format& fmt, const uint32_t pixelFormat) override {fmt.fmt.pix.pixelformat = pixelFormat;}
};

class PixFormatMPlane : public IPixFormat
{
public:
    virtual
    uint32_t GetSizeImage(const v4l2_format& fmt) override {return fmt.fmt.pix_mp.plane_fmt[0].sizeimage;}

    virtual
    uint32_t GetWidth(const v4l2_format& fmt) override {return fmt.fmt.pix_mp.width;}

    virtual
    uint32_t GetHeight(const v4l2_format& fmt) override {return fmt.fmt.pix_mp.height;}

    virtual
    uint32_t GetField(const v4l2_format& fmt) override {return fmt.fmt.pix_mp.field;}

    virtual
    uint32_t GetBytesPerLine(const v4l2_format& fmt) override {return fmt.fmt.pix_mp.plane_fmt[0].bytesperline;}

    virtual
    uint32_t GetPixelFormat(const v4l2_format& fmt) override {return fmt.fmt.pix_mp.pixelformat;}

    virtual
    void SetSizeImage(v4l2_format& fmt, const uint32_t sizeImage) override {fmt.fmt.pix_mp.plane_fmt[0].sizeimage = sizeImage;}

    virtual
    void SetWidth(v4l2_format& fmt, const uint32_t width) override {fmt.fmt.pix_mp.width = width;}

    virtual
    void SetHeight(v4l2_format& fmt, const uint32_t height) override {fmt.fmt.pix_mp.height = height;}

    virtual
    void SetField(v4l2_format& fmt, const uint32_t field) override {fmt.fmt.pix_mp.field = field;}

    virtual
    void SetBytesPerLine(v4l2_format& fmt, const uint32_t bytesPerLine) override {fmt.fmt.pix_mp.plane_fmt[0].bytesperline = bytesPerLine;}

    virtual
    void SetPixelFormat(v4l2_format& fmt, const uint32_t pixelFormat) override {fmt.fmt.pix_mp.pixelformat = pixelFormat;}
};

Q_DECLARE_METATYPE(v4l2_event_ctrl);

Camera::Camera()
    : m_DeviceFileDescriptor(-1)
    , m_SubDeviceFileDescriptors()
    , m_ControlIdToFileDescriptorMap()
    , m_BlockingMode(false)
    , m_ShowFrames(true)
    , m_UseV4L2TryFmt(true)
    , m_Recording(false)
    , m_IsAvtCamera(true)
    , m_DeviceBufferType(V4L2_BUF_TYPE_VIDEO_CAPTURE)
    , m_SubDeviceBufferTypes()
    , m_FrameRateDeviceFileDescriptor(-1)
    , m_CropDeviceFileDescriptor(-1)
    , m_pPixFormat(nullptr)
    , m_pEventHandler(nullptr)
{
    connect(&m_CameraObserver, SIGNAL(OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &, const QString &)), this, SLOT(OnCameraListChanged(const int &, unsigned int, unsigned long long, const QString &, const QString &)));

    connect(&m_CameraObserver, SIGNAL(OnSubDeviceListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &, const QString &)), this, SLOT(OnSubDeviceListChanged(const int &, unsigned int, unsigned long long, const QString &, const QString &)));

    m_pAutoExposureReader = new AutoReader();
    connect(m_pAutoExposureReader->GetAutoReaderWorker(), SIGNAL(ReadSignal()), this, SLOT(PassExposureValue()), Qt::DirectConnection);
    m_pAutoExposureReader->MoveToThreadAndStart();

    m_pAutoGainReader = new AutoReader();
    connect(m_pAutoGainReader->GetAutoReaderWorker(), SIGNAL(ReadSignal()), this, SLOT(PassGainValue()), Qt::DirectConnection);
    m_pAutoGainReader->MoveToThreadAndStart();

}

Camera::~Camera()
{
    delete m_pAutoExposureReader;
    m_pAutoExposureReader = nullptr;
    delete m_pAutoGainReader;
    m_pAutoGainReader = nullptr;

    m_CameraObserver.SetTerminateFlag();
    if (NULL != m_pFrameObserver.data())
        m_pFrameObserver->StopStream();




    if(m_pPixFormat)
    {
        delete m_pPixFormat;
        m_pPixFormat = nullptr;
    }

    CloseDevice();
}

double Camera::GetReceivedFPS()
{
    return m_pFrameObserver->GetReceivedFPS();
}

double Camera::GetRenderedFPS()
{
    return m_pFrameObserver->GetRenderedFPS();
}

int Camera::OpenDevice(std::string &deviceName, QVector<QString>& subDevices, bool blockingMode, IO_METHOD_TYPE ioMethodType,
               bool v4l2TryFmt)
{
    int result = -1;

    m_BlockingMode = blockingMode;
    m_UseV4L2TryFmt = v4l2TryFmt;

    switch (ioMethodType)
    {
    case IO_METHOD_MMAP:
         m_pFrameObserver = QSharedPointer<FrameObserverMMAP>(new FrameObserverMMAP(m_ShowFrames));
         break;
    case IO_METHOD_USERPTR:
         m_pFrameObserver = QSharedPointer<FrameObserverUSER>(new FrameObserverUSER(m_ShowFrames));
         break;
    }
    connect(m_pFrameObserver.data(), SIGNAL(OnFrameReady_Signal(const QImage &, const unsigned long long &)), this, SLOT(OnFrameReady(const QImage &, const unsigned long long &)));
    connect(m_pFrameObserver.data(), SIGNAL(OnFrameID_Signal(const unsigned long long &)), this, SLOT(OnFrameID(const unsigned long long &)));
    connect(m_pFrameObserver.data(), SIGNAL(OnDisplayFrame_Signal(const unsigned long long &)), this, SLOT(OnDisplayFrame(const unsigned long long &)));

    if (-1 == m_DeviceFileDescriptor)
    {
        if (m_BlockingMode)
            m_DeviceFileDescriptor = open(deviceName.c_str(), O_RDWR, 0);
        else
            m_DeviceFileDescriptor = open(deviceName.c_str(), O_RDWR | O_NONBLOCK, 0);

        m_FileDescriptorToNameMap[m_DeviceFileDescriptor] = deviceName;

        LOG_EX("Camera::OpenDevice device %s has file descriptor %d", deviceName.c_str(), m_DeviceFileDescriptor);

        v4l2_capability cap;
        if (-1 == iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_QUERYCAP, &cap))
        {
            LOG_EX("Camera::OpenDevice no V4L2 device");
        }
        else
        {
            m_DeviceBufferType = (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ? V4L2_BUF_TYPE_VIDEO_CAPTURE : (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) ? V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE : V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (m_DeviceBufferType == V4L2_BUF_TYPE_VIDEO_CAPTURE)
            {
                m_pPixFormat = new PixFormat();
                LOG_EX("Camera::OpenDevice %s is a single-plane video capture device", deviceName.c_str());
            }
            else if (m_DeviceBufferType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
            {
                m_pPixFormat = new PixFormatMPlane();
                LOG_EX("Camera::OpenDevice %s is a multi-plane video capture device", deviceName.c_str());
            }
            else
            {
                LOG_EX("Camera::OpenDevice %s is no video capture device", deviceName.c_str());
            }
        }

        QueryControls(m_DeviceFileDescriptor);

        if (m_DeviceFileDescriptor == -1)
        {
            LOG_EX("Camera::OpenDevice open %s failed errno=%d=%s", deviceName.c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
        else
        {
            getAvtDeviceFirmwareVersion();

            LOG_EX("Camera::OpenDevice open %s OK", deviceName.c_str());

            m_DeviceName = deviceName;

            result = 0;

            m_pEventHandler = new V4L2EventHandler(m_DeviceFileDescriptor);

			qRegisterMetaType<v4l2_event_ctrl>();

            connect(m_pEventHandler,SIGNAL(ControlChanged(int,v4l2_event_ctrl)),this,SLOT(OnCtrlUpdate(int, v4l2_event_ctrl)));

			m_pEventHandler->start();
        }

        for (auto const subDevice : subDevices)
        {
            int subDeviceFileDescriptor = open(subDevice.toStdString().c_str(), O_RDWR | O_NONBLOCK, 0);
            if (subDeviceFileDescriptor == -1)
            {
                LOG_EX("Camera::OpenDevice open sub-device '%s' failed errno=%d=%s", subDevice.toStdString().c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
            else
            {
                LOG_EX("Camera::OpenDevice sub-device %s has file descriptor %d", subDevice.toStdString().c_str(), subDeviceFileDescriptor);
                m_SubDeviceFileDescriptors.push_back(subDeviceFileDescriptor);

                m_FileDescriptorToNameMap[subDeviceFileDescriptor] = subDevice.toStdString();
            }

            if (-1 == iohelper::xioctl(subDeviceFileDescriptor, VIDIOC_QUERYCAP, &cap))
            {
                LOG_EX("Camera::OpenDevice no V4L2 device");
            }
            else
            {
                m_SubDeviceBufferTypes[subDeviceFileDescriptor] = (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ? V4L2_BUF_TYPE_VIDEO_CAPTURE : (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) ? V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE : V4L2_BUF_TYPE_VIDEO_CAPTURE;

                if (m_SubDeviceBufferTypes[subDeviceFileDescriptor] == V4L2_BUF_TYPE_VIDEO_CAPTURE)
                {
                    LOG_EX("Camera::OpenDevice %s is a single-plane video capture device", subDevice.toStdString().c_str());
                }
                else if (m_SubDeviceBufferTypes[subDeviceFileDescriptor] == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
                {
                    LOG_EX("Camera::OpenDevice %s is a multi-plane video capture device", subDevice.toStdString().c_str());
                }
                else
                {
                    LOG_EX("Camera::OpenDevice %s is no video capture device", subDevice.toStdString().c_str());
                }
            }

            QueryControls(subDeviceFileDescriptor);
        }
    }
    else
    {
        LOG_EX("Camera::OpenDevice open %s failed because %s is already open", deviceName.c_str(), m_DeviceName.c_str());
    }

    return result;
}

int Camera::CloseDevice()
{
    int result = -1;

    if (m_pEventHandler != nullptr)
    {
		m_pEventHandler->Stop();

        delete m_pEventHandler;
        m_pEventHandler = nullptr;
    }

    if (-1 != m_DeviceFileDescriptor)
    {
        if (-1 == close(m_DeviceFileDescriptor))
        {
            LOG_EX("Camera::CloseDevice close %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
        else
        {
            LOG_EX("Camera::CloseDevice close %s OK", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str());
            result = 0;
        }
    }

    m_DeviceFileDescriptor = -1;

    for (auto const subDeviceFileDescriptor : m_SubDeviceFileDescriptors)
    {
        if (-1 == close(subDeviceFileDescriptor))
        {
            LOG_EX("Camera::CloseDevice sub-device close %s failed errno=%d=%s", m_FileDescriptorToNameMap[subDeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
        else
        {
            LOG_EX("Camera::CloseDevice sub-device close %s OK", m_FileDescriptorToNameMap[subDeviceFileDescriptor].c_str());
            result = 0;
        }
    }
    m_SubDeviceFileDescriptors.clear();
    m_SubDeviceBufferTypes.clear();

    m_FileDescriptorToNameMap.clear();

    return result;
}

void Camera::QueryControls(int fd)
{
    const unsigned next_fl = V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_NEXT_COMPOUND;

    LOG_EX("Camera::QueryControls querying controls for %s", m_FileDescriptorToNameMap[fd].c_str());
    v4l2_queryctrl qctrl;
    qctrl.id = next_fl;
    while (iohelper::xioctl(fd, VIDIOC_QUERYCTRL, &qctrl) == 0)
    {
        LOG_EX("Camera::QueryControls id=%d,name=%s,flags=%d,min=%d,max=%d,step=%d,type=%d,defval=%d", qctrl.id, qctrl.name, qctrl.flags, qctrl.minimum, qctrl.maximum, qctrl.step, qctrl.type, qctrl.default_value);
        qctrl.id |= next_fl;
    }

    LOG_EX("Camera::QueryControls querying control menu");
    v4l2_querymenu qctrlmenu;
    qctrlmenu.id = next_fl;
    while (iohelper::xioctl(fd, VIDIOC_QUERYMENU, &qctrlmenu) == 0)
    {
        LOG_EX("Camera::QueryControls id=%d,index=%d,name=%s,value=%d", qctrlmenu.id, qctrlmenu.index, qctrlmenu.name, qctrlmenu.value);
        qctrlmenu.id |= next_fl;
    }

    LOG_EX("Camera::QueryControls querying extended controls");
    v4l2_query_ext_ctrl qctrl_ext;
    qctrl_ext.id = next_fl;
    while (iohelper::xioctl(fd, VIDIOC_QUERY_EXT_CTRL, &qctrl_ext) == 0)
    {
        LOG_EX("Camera::QueryControls id=%d,name=%s,flags=%d,min=%d,max=%d,step=%d,type=%d,defval=%d,elsz=%d,elms=%d,ndims=%d,dim0=%d,dim1=%d,dim2=%d,dim3=%d", qctrl_ext.id, qctrl_ext.name, qctrl_ext.flags, qctrl_ext.minimum, qctrl_ext.maximum, qctrl_ext.step, qctrl_ext.type, qctrl_ext.default_value, qctrl_ext.elem_size, qctrl_ext.elems, qctrl_ext.nr_of_dims, qctrl_ext.dims[0], qctrl_ext.dims[1], qctrl_ext.dims[2], qctrl_ext.dims[3]);
        qctrl_ext.id |= next_fl;
    }
}

/********************************************************************************/
// slots
/********************************************************************************/

// The event handler to show the processed frame
void Camera::OnFrameReady(const QImage &image, const unsigned long long &frameId)
{
    emit OnCameraFrameReady_Signal(image, frameId);
}

// The event handler to show the processed frame ID
void Camera::OnFrameID(const unsigned long long &frameId)
{
    emit OnCameraFrameID_Signal(frameId);
}

// Event will be called when a frame is displayed
void Camera::OnDisplayFrame(const unsigned long long &frameID)
{
    emit OnCameraDisplayFrame_Signal(frameID);
}

// The event handler to set or remove devices
void Camera::OnCameraListChanged(const int &reason, unsigned int cardNumber, unsigned long long deviceID, const QString &deviceName, const QString &info)
{
    emit OnCameraListChanged_Signal(reason, cardNumber, deviceID, deviceName, info);
}

// The event handler to set or remove sub-devices
void Camera::OnSubDeviceListChanged(const int &reason, unsigned int cardNumber, unsigned long long deviceID, const QString &deviceName, const QString &info)
{
    emit OnSubDeviceListChanged_Signal(reason, cardNumber, deviceID, deviceName, info);
}

/********************************************************************************/
// Device discovery
/********************************************************************************/

int Camera::DeviceDiscoveryStart()
{
    uint32_t deviceCount = 0;
    QString deviceName;
    int fileDiscriptor = -1;

    do
    {
        deviceName.sprintf("/dev/video%d", deviceCount);

        if ((fileDiscriptor = open(deviceName.toStdString().c_str(), O_RDWR)) == -1)
        {
            LOG_EX("Camera::DeviceDiscoveryStart open %s failed errno=%d=%s", deviceName.toLatin1().data(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
        else
        {
            v4l2_capability cap;

            LOG_EX("Camera::DeviceDiscoveryStart open %s found", deviceName.toLatin1().data());

            // query device capabilities
            if (-1 == iohelper::xioctl(fileDiscriptor, VIDIOC_QUERYCAP, &cap))
            {
                LOG_EX("Camera::DeviceDiscoveryStart %s is no V4L2 device errno=%d=%s", deviceName.toLatin1().data(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
            else
            {
                if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
                {
                    LOG_EX("CameraObserver::DeviceDiscoveryStart %s is a single-plane video capture device", deviceName.toLatin1().data());
                }
                else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                {
                    LOG_EX("CameraObserver::DeviceDiscoveryStart %s is a multi-plane video capture device", deviceName.toLatin1().data());
                }

                if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
                {
                    LOG_EX("Camera::DeviceDiscoveryStart %s is no video capture device", deviceName.toLatin1().data());
                }
                else
                {
                    emit OnCameraListChanged_Signal(UpdateTriggerPluggedIn, 0, deviceCount, deviceName, (const char*) cap.card);
                }
            }


            if (-1 == close(fileDiscriptor))
            {
                LOG_EX("Camera::DeviceDiscoveryStart close %s failed errno=%d=%s", deviceName.toLatin1().data(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
            else
            {
                LOG_EX("Camera::DeviceDiscoveryStart close %s OK", deviceName.toLatin1().data());
            }
        }

        deviceCount++;
    }
    while(deviceCount < 20);

    return 0;
}

int Camera::DeviceDiscoveryStop()
{
    LOG_EX("Device Discovery stopped.");

    return 0;
}

int Camera::SubDeviceDiscoveryStart()
{
    uint32_t subDeviceCount = 0;
    QString subDeviceName;
    int fileDiscriptor = -1;

    do
    {
        subDeviceName.sprintf("/dev/v4l-subdev%d", subDeviceCount);

        if ((fileDiscriptor = open(subDeviceName.toStdString().c_str(), O_RDWR)) == -1)
        {
            LOG_EX("Camera::SubDeviceDiscoveryStart open %s failed errno=%d=%s", subDeviceName.toLatin1().data(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
        else
        {
            v4l2_capability cap;

            LOG_EX("Camera::SubDeviceDiscoveryStart open %s found", subDeviceName.toLatin1().data());

            // query sub-device capabilities
            if (-1 == iohelper::xioctl(fileDiscriptor, VIDIOC_QUERYCAP, &cap))
            {
                LOG_EX("Camera::SubDeviceDiscoveryStart %s is no V4L2 device errno=%d=%s", subDeviceName.toLatin1().data(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
            else
            {
                if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
                {
                    LOG_EX("CameraObserver::SubDeviceDiscoveryStart %s is a single-plane video capture device", subDeviceName.toLatin1().data());
                }
                else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                {
                    LOG_EX("CameraObserver::SubDeviceDiscoveryStart %s is a multi-plane video capture device", subDeviceName.toLatin1().data());
                }

                if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
                {
                    LOG_EX("Camera::SubDeviceDiscoveryStart %s is no video capture device", subDeviceName.toLatin1().data());
                }
                else
                {
                    emit OnSubDeviceListChanged_Signal(UpdateTriggerPluggedIn, 0, subDeviceCount, subDeviceName, (const char*) cap.card);
                }
            }

            if (-1 == close(fileDiscriptor))
            {
                LOG_EX("Camera::SubDeviceDiscoveryStart close %s failed errno=%d=%s", subDeviceName.toLatin1().data(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
            else
            {
                LOG_EX("Camera::SubDeviceDiscoveryStart close %s OK", subDeviceName.toLatin1().data());
            }
        }

        subDeviceCount++;
    } while (subDeviceCount < 20);

    return 0;
}

int Camera::SubDeviceDiscoveryStop()
{
    LOG_EX("Sub-device discovery stopped.");

    return 0;
}

/********************************************************************************/
// SI helper
/********************************************************************************/

int Camera::StartStreamChannel(uint32_t pixelFormat, uint32_t payloadSize, uint32_t width, uint32_t height,
                               uint32_t bytesPerLine, void *pPrivateData,
                               uint32_t enableLogging)
{
    int nResult = 0;

    LOG_EX("Camera::StartStreamChannel %s pixelFormat=%d, payloadSize=%d, width=%d, height=%d.", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), pixelFormat, payloadSize, width, height);

    m_pFrameObserver->StartStream(m_BlockingMode, m_DeviceFileDescriptor, pixelFormat,
                                  payloadSize, width, height, bytesPerLine,
                                  enableLogging);


    // start stream returns always success
    LOG_EX("Camera::StartStreamChannel OK.");

    return nResult;
}

int Camera::StopStreamChannel()
{
    int nResult = 0;

	LOG_EX("Camera::StopStreamChannel started.");

    nResult = m_pFrameObserver->StopStream();

    if (nResult == 0)
    {
        LOG_EX("Camera::StopStreamChannel OK.");
    }
    else
    {
        LOG_EX("Camera::StopStreamChannel failed.");
    }

    m_CsvData.resize(0);


    return nResult;
}

/*********************************************************************************************************/
// DCI commands
/*********************************************************************************************************/

int Camera::StartStreaming()
{
    int result = -1;
    v4l2_buf_type type;

    type = m_DeviceBufferType;

    if (-1 == iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_STREAMON, &type))
    {
        LOG_EX("Camera::StartStreaming VIDIOC_STREAMON %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }
    else
    {
        LOG_EX("Camera::StartStreaming VIDIOC_STREAMON OK");

        result = 0;
    }

    return result;
}

int Camera::StopStreaming()
{
    int result = -1;
    v4l2_buf_type type;

    type = m_DeviceBufferType;

    if (-1 == iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_STREAMOFF, &type))
    {
        LOG_EX("Camera::StopStreaming VIDIOC_STREAMOFF %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }
    else
    {
        LOG_EX("Camera::StopStreaming VIDIOC_STREAMOFF OK");

        result = 0;
    }

    return result;
}

int Camera::ReadPayloadSize(uint32_t &payloadSize)
{
    int result = -1;
    v4l2_format fmt;

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;

    if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
    {
        payloadSize = m_pPixFormat->GetSizeImage(fmt);

        LOG_EX("Camera::ReadPayloadSize VIDIOC_G_FMT %s OK =%d", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), payloadSize);

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadPayloadSize VIDIOC_G_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::ReadFrameSize(uint32_t &width, uint32_t &height)
{
    int result = -1;
    v4l2_format fmt;

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;

    if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
    {
        width = m_pPixFormat->GetWidth(fmt);
        height = m_pPixFormat->GetHeight(fmt);

        LOG_EX("Camera::ReadFrameSize VIDIOC_G_FMT %s OK =%dx%d", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), width, height);

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadFrameSize VIDIOC_G_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::SetFrameSize(uint32_t width, uint32_t height)
{
    int result = -1;
    v4l2_format fmt;

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;

    if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
    {
        LOG_EX("Camera::SetFrameSize VIDIOC_G_FMT OK");

        m_pPixFormat->SetWidth(fmt, width);
        m_pPixFormat->SetHeight(fmt, height);
        m_pPixFormat->SetField(fmt, V4L2_FIELD_ANY);
        m_pPixFormat->SetBytesPerLine(fmt, 0);

        if (m_UseV4L2TryFmt)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_TRY_FMT, &fmt);
            if (result != 0)
            {
                LOG_EX("Camera::SetFrameSize VIDIOC_TRY_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
        }
        else
            result = 0;

        if (0 == result)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_S_FMT, &fmt);
            if (-1 != result)
            {
                LOG_EX("Camera::SetFrameSize VIDIOC_S_FMT %s OK =%dx%d", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), width, height);

                result = 0;
            }
            else
            {
                LOG_EX("Camera::SetFrameSize VIDIOC_S_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
        }
    }
    else
    {
        LOG_EX("Camera::SetFrameSize VIDIOC_G_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::SetWidth(uint32_t width)
{
    int result = -1;
    v4l2_format fmt;

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;

    if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
    {
        LOG_EX("Camera::SetWidth VIDIOC_G_FMT %s OK", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str());

        m_pPixFormat->SetWidth(fmt, width);
        m_pPixFormat->SetField(fmt, V4L2_FIELD_ANY);
        m_pPixFormat->SetBytesPerLine(fmt, 0);

        if (m_UseV4L2TryFmt)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_TRY_FMT, &fmt);
            if (result != 0)
            {
                LOG_EX("Camera::SetWidth VIDIOC_TRY_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
            else
            {
                LOG_EX("Camera::SetWidth VIDIOC_TRY_FMT %s OK", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str());
            }
        }
        else
        {
            LOG_EX("Camera::SetWidth VIDIOC_TRY_FMT %s not used", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str());
            result = 0;
        }

        if (0 == result)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_S_FMT, &fmt);
            if (-1 != result)
            {
                LOG_EX("Camera::SetWidth VIDIOC_S_FMT %s OK =%d", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), width);

                result = 0;
            }
            else
            {
                LOG_EX("Camera::SetWidth VIDIOC_S_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
        }
    }
    else
    {
        LOG_EX("Camera::SetWidth VIDIOC_G_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::ReadWidth(uint32_t &width)
{
    int result = -1;
    v4l2_format fmt;

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;

    if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
    {
        width = m_pPixFormat->GetWidth(fmt);

        LOG_EX("Camera::ReadWidth VIDIOC_G_FMT %s OK =%d", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), width);

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadWidth VIDIOC_G_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::SetHeight(uint32_t height)
{
    int result = -1;
    v4l2_format fmt;

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;

    if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
    {
        LOG_EX("Camera::SetHeight VIDIOC_G_FMT %s OK", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str());

        m_pPixFormat->SetHeight(fmt, height);
        m_pPixFormat->SetField(fmt, V4L2_FIELD_ANY);
        m_pPixFormat->SetBytesPerLine(fmt, 0);

        if (m_UseV4L2TryFmt)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_TRY_FMT, &fmt);
            if (result != 0)
            {
                LOG_EX("Camera::SetHeight VIDIOC_TRY_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
            else
            {
                LOG_EX("Camera::SetHeight VIDIOC_TRY_FMT %s OK", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str());
            }
        }
        else
        {
            LOG_EX("Camera::SetHeight VIDIOC_TRY_FMT %s not used", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str());

            result = 0;
        }

        if (0 == result)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_S_FMT, &fmt);
            if (-1 != result)
            {
                LOG_EX("Camera::SetHeight VIDIOC_S_FMT %s OK =%d", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), height);

                result = 0;
            }
            else
            {
                LOG_EX("Camera::SetHeight VIDIOC_S_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
        }
    }
    else
    {
        LOG_EX("Camera::SetHeight VIDIOC_G_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::ReadHeight(uint32_t &height)
{
    int result = -1;
    v4l2_format fmt;

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;

    if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
    {
        height = m_pPixFormat->GetHeight(fmt);

        LOG_EX("Camera::ReadHeight VIDIOC_G_FMT OK =%d", height);

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadHeight VIDIOC_G_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::ReadFormats()
{
    int result = -1;
    v4l2_fmtdesc fmt;
    v4l2_frmsizeenum fmtsize;

    LOG_EX("Camera::ReadFormats %s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str());

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;
    while (iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_ENUM_FMT, &fmt) >= 0 && fmt.index <= 100)
    {
        std::string tmp = (char*)fmt.description;
        LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FMT index = %d type = %d pixel format = %d = %s description = %s", fmt.index, fmt.type, fmt.pixelformat, v4l2helper::ConvertPixelFormat2EnumString(fmt.pixelformat).c_str(), fmt.description);

        emit OnCameraPixelFormat_Signal(QString("%1").arg(QString(v4l2helper::ConvertPixelFormat2String(fmt.pixelformat).c_str())));

        CLEAR(fmtsize);
        fmtsize.type = m_DeviceBufferType;
        fmtsize.pixel_format = fmt.pixelformat;
        fmtsize.index = 0;
        while (iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_ENUM_FRAMESIZES, &fmtsize) >= 0 && fmtsize.index <= 100)
        {
            if (fmtsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                v4l2_frmivalenum fmtival;

                LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum discrete width = %d height = %d", fmtsize.discrete.width, fmtsize.discrete.height);

                //emit OnCameraFrameSize_Signal(QString("disc:%1x%2").arg(fmtsize.discrete.width).arg(fmtsize.discrete.height));

                CLEAR(fmtival);
                fmtival.index = 0;
                fmtival.pixel_format = fmt.pixelformat;
                fmtival.width = fmtsize.discrete.width;
                fmtival.height = fmtsize.discrete.height;
                while (iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_ENUM_FRAMEINTERVALS, &fmtival) >= 0)
                {
                    fmtival.index++;
                }
            }
            else if (fmtsize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
            {
                LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise min_width = %d min_height = %d max_width = %d max_height = %d step_width = %d step_height = %d",
                        fmtsize.stepwise.min_width, fmtsize.stepwise.min_height, fmtsize.stepwise.max_width, fmtsize.stepwise.max_height, fmtsize.stepwise.step_width, fmtsize.stepwise.step_height);

                //emit OnCameraFrameSize_Signal(QString("min:%1x%2,max:%3x%4,step:%5x%6").arg(fmtsize.stepwise.min_width).arg(fmtsize.stepwise.min_height).arg(fmtsize.stepwise.max_width).arg(fmtsize.stepwise.max_height).arg(fmtsize.stepwise.step_width).arg(fmtsize.stepwise.step_height));
            }

            result = 0;

            fmtsize.index++;
        }

        if (fmtsize.index >= 100)
        {
            LOG_EX("Camera::ReadFormats no VIDIOC_ENUM_FRAMESIZES never terminated with EINVAL within 100 loops.");
        }

        fmt.index++;
    }

    if (fmt.index >= 100)
    {
        LOG_EX("Camera::ReadFormats no VIDIOC_ENUM_FMT never terminated with EINVAL within 100 loops.");
    }

    return result;
}

int Camera::SetPixelFormat(uint32_t pixelFormat, QString pfText)
{
    int result = -1;
    v4l2_format fmt;

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;

    if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
    {
        LOG_EX("Camera::SetPixelFormat VIDIOC_G_FMT %s OK", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str());

        m_pPixFormat->SetPixelFormat(fmt, pixelFormat);
        m_pPixFormat->SetField(fmt, V4L2_FIELD_ANY);
        m_pPixFormat->SetBytesPerLine(fmt, 0);

        if (m_UseV4L2TryFmt)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_TRY_FMT, &fmt);
            if (result != 0)
            {
                LOG_EX("Camera::SetPixelFormat VIDIOC_TRY_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
            else
            {
                LOG_EX("Camera::SetPixelFormat VIDIOC_TRY_FMT %s OK", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str());
            }
        }
        else
        {
            result = 0;
        }

        if (0 == result)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_S_FMT, &fmt);
            if (-1 != result)
            {
                LOG_EX("Camera::SetPixelFormat VIDIOC_S_FMT %s to %d OK", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), pixelFormat);
                result = 0;
            }
            else
            {
                LOG_EX("Camera::SetPixelFormat VIDIOC_S_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
        }

    }
    else
    {
        LOG_EX("Camera::SetPixelFormat VIDIOC_G_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

QList<QString> Camera::GetFrameSizes(uint32_t fourcc)
{
	int result = -1,index = 0;
	v4l2_frmsizeenum frmsizeenum;
	QList<QString> framesizes;

	frmsizeenum.pixel_format = fourcc;
	frmsizeenum.index = index;

	while (!iohelper::xioctl(m_DeviceFileDescriptor,VIDIOC_ENUM_FRAMESIZES,&frmsizeenum)) {
		framesizes.append(QString("%1x%2").arg(frmsizeenum.discrete.width).arg(frmsizeenum.discrete.height));


		frmsizeenum.index = (++index);
	}

	return framesizes;
}

int Camera::GetFrameSizeIndex()
{
	v4l2_format fmt;
	v4l2_frmsizeenum frmsizeenum;

	CLEAR(fmt);
	fmt.type = m_DeviceBufferType;

	if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
	{
		int index = 0;

		frmsizeenum.pixel_format = fmt.fmt.pix.pixelformat;
		frmsizeenum.index = index;

		v4l2_selection sel;
		sel.target = V4L2_SEL_TGT_NATIVE_SIZE;
		sel.type = m_DeviceBufferType;

		if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_SELECTION, &sel)) {

			while (!iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_ENUM_FRAMESIZES, &frmsizeenum))
			{
				if (frmsizeenum.discrete.width == sel.r.width &&
					frmsizeenum.discrete.height == sel.r.height)
					return index;

				frmsizeenum.index = (++index);
			}
		}
	}

	return -1;
}

void Camera::SetFrameSizeByIndex(int index)
{
	v4l2_format fmt;
	v4l2_frmsizeenum frmsizeenum;

	CLEAR(fmt);
	fmt.type = m_DeviceBufferType;

	if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
	{
		frmsizeenum.pixel_format = fmt.fmt.pix.pixelformat;
		frmsizeenum.index = index;

		if (!iohelper::xioctl(m_DeviceFileDescriptor,VIDIOC_ENUM_FRAMESIZES,&frmsizeenum)) {
			fmt.fmt.pix.width = frmsizeenum.discrete.width;
			fmt.fmt.pix.height = frmsizeenum.discrete.height;

			iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_S_FMT, &fmt);
		}
	}
}

int Camera::ReadPixelFormat(uint32_t &pixelFormat, uint32_t &bytesPerLine, QString &pfText)
{
    int result = -1;
    v4l2_format fmt;

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;

    if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
    {
        LOG_EX("Camera::ReadPixelFormat VIDIOC_G_FMT %s OK =%d", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), fmt.fmt.pix.pixelformat);

        pixelFormat = m_pPixFormat->GetPixelFormat(fmt);
        bytesPerLine = m_pPixFormat->GetBytesPerLine(fmt);
        pfText = QString(v4l2helper::ConvertPixelFormat2EnumString(m_pPixFormat->GetPixelFormat(fmt)).c_str());

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadPixelFormat VIDIOC_G_FMT %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

//////////////////// Extended Controls ////////////////////////

int Camera::EnumAllControlNewStyle()
{
    int result = -1;

    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        v4l2_query_ext_ctrl qctrl;
        v4l2_querymenu queryMenu;

        int cidCount = 0;

        CLEAR(qctrl);
        CLEAR(queryMenu);

        qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

        while (0 == iohelper::xioctl(fileDescriptor, VIDIOC_QUERY_EXT_CTRL, &qctrl))
        {
            if (!(qctrl.flags & V4L2_CTRL_FLAG_DISABLED))
            {
                bool bIsReadOnly = false;
                if (qctrl.flags & V4L2_CTRL_FLAG_READ_ONLY || qctrl.id == V4L2_CID_PREFFERED_STRIDE)
                {
                    bIsReadOnly = true;
                    qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
                    continue;
                }

				if (qctrl.flags & V4L2_CTRL_FLAG_INACTIVE || qctrl.flags & V4L2_CTRL_FLAG_GRABBED)
				{
					bIsReadOnly = true;
				}

				if (qctrl.flags & V4L2_CTRL_FLAG_VOLATILE && (qctrl.flags & V4L2_CTRL_FLAG_EXECUTE_ON_WRITE) == 0)
				{
					bIsReadOnly = true;
				}

				if (m_pEventHandler != nullptr) {
					m_pEventHandler->SubscribeControl(qctrl.id);
				}

                LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s id=%d=%s min=%ld, max=%ld, default=%ld", m_FileDescriptorToNameMap[fileDescriptor].c_str(), qctrl.id, v4l2helper::ConvertControlID2String(qctrl.id).c_str(), qctrl.minimum, qctrl.maximum, qctrl.default_value);
                cidCount++;

                QString name = QString((const char*) qctrl.name);
                if(UsesSubdevices()) {
                    name += QString(" (") + (fileDescriptor == m_DeviceFileDescriptor ? "d" : "s") + ")";
                }

                QString unit = QString::fromStdString(v4l2helper::GetControlUnit(qctrl.id));

                if (qctrl.type == V4L2_CTRL_TYPE_INTEGER)
                {
                    int result = -1;
                    int32_t value;
                    int32_t max = qctrl.maximum;
                    int32_t min = qctrl.minimum;
                    int32_t id = qctrl.id;

                    result = ReadExtControl(fileDescriptor, value, id, "ReadEnumerationControl", "V4L2_CTRL_TYPE_INTEGER", V4L2_CTRL_ID2CLASS(qctrl.id));

                    if (result == 0)
                    {
                        LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s will be used for %s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), qctrl.name);
                        m_ControlIdToControlNameMap[qctrl.id] = qctrl.name;
                        m_ControlIdToFileDescriptorMap[qctrl.id] = fileDescriptor;
                        emit SendIntDataToEnumerationWidget(id, min, max, value, name, unit, bIsReadOnly);
                    }
                }
                else if (qctrl.type == V4L2_CTRL_TYPE_INTEGER64)
                {
                    int result = -1;
                    int64_t value;
                    int64_t max = qctrl.maximum;
                    int64_t min = qctrl.minimum;
                    int32_t id = qctrl.id;

                    result = ReadExtControl(fileDescriptor, value, id, "ReadEnumerationControl", "V4L2_CTRL_TYPE_INTEGER64", V4L2_CTRL_ID2CLASS(qctrl.id));

                    if (result == 0)
                    {
                        LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s will be used for %s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), qctrl.name);
                        m_ControlIdToControlNameMap[qctrl.id] = qctrl.name;
                        m_ControlIdToFileDescriptorMap[qctrl.id] = fileDescriptor;
                        emit SentInt64DataToEnumerationWidget(id, min, max, value, name, unit, bIsReadOnly);
                    }
                }
                else if (qctrl.type == V4L2_CTRL_TYPE_BOOLEAN)
                {
                    int result = -1;
                    int32_t value;
                    int32_t id = qctrl.id;

                    result = ReadExtControl(fileDescriptor, value, id, "ReadEnumerationControl", "V4L2_CTRL_TYPE_BOOLEAN", V4L2_CTRL_ID2CLASS(qctrl.id));

                    if (result == 0)
                    {
                        LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s will be used for %s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), qctrl.name);
                        m_ControlIdToControlNameMap[qctrl.id] = qctrl.name;
                        m_ControlIdToFileDescriptorMap[qctrl.id] = fileDescriptor;
                        emit SendBoolDataToEnumerationWidget(id, static_cast<bool>(value), name, unit, bIsReadOnly);
                    }
                }
                else if (qctrl.type == V4L2_CTRL_TYPE_BUTTON)
                {
                    int32_t id = qctrl.id;
                    LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s will be used for %s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), qctrl.name);
                    m_ControlIdToControlNameMap[qctrl.id] = qctrl.name;
                    m_ControlIdToFileDescriptorMap[qctrl.id] = fileDescriptor;
                    emit SendButtonDataToEnumerationWidget(id, name, unit, bIsReadOnly);
                }
                else if (qctrl.type == V4L2_CTRL_TYPE_MENU)
                {
                    int result = -1;
                    int32_t id = qctrl.id;
                    int32_t value;
                    QList<QString> list;

                    result = ReadExtControl(fileDescriptor, value, id, "ReadEnumerationControl", "V4L2_CTRL_TYPE_MENU", V4L2_CTRL_ID2CLASS(qctrl.id));

                    if (result == 0)
                    {
                        queryMenu.id = qctrl.id;
                        for (queryMenu.index = qctrl.minimum; queryMenu.index <= qctrl.maximum; queryMenu.index++)
                        {

                            if (0 == iohelper::xioctl(fileDescriptor,
                            VIDIOC_QUERYMENU, &queryMenu))
                            {
                                list.append(QString((const char*) queryMenu.name));
                            }
                        }
                        LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s will be used for %s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), qctrl.name);
                        m_ControlIdToControlNameMap[qctrl.id] = qctrl.name;
                        m_ControlIdToFileDescriptorMap[qctrl.id] = fileDescriptor;
                        emit SendListDataToEnumerationWidget(id, value, list, name, unit, bIsReadOnly);
                    }
                }
                else if (qctrl.type == V4L2_CTRL_TYPE_INTEGER_MENU)
                {
                    int result = -1;
                    int32_t id = qctrl.id;
                    int32_t value;
                    QList<int64_t> list;

                    result = ReadExtControl(fileDescriptor, value, id, "ReadEnumerationControl", "V4L2_CTRL_TYPE_INTEGER_MENU", V4L2_CTRL_ID2CLASS(qctrl.id));

                    if (result == 0)
                    {
                        queryMenu.id = qctrl.id;
                        for (queryMenu.index = qctrl.minimum; queryMenu.index <= qctrl.maximum; queryMenu.index++)
                        {
                            if (0 == iohelper::xioctl(fileDescriptor,
                            VIDIOC_QUERYMENU, &queryMenu))
                            {
                                list.append(queryMenu.value);
                            }
                        }
                        LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s will be used for %s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), qctrl.name);
                        m_ControlIdToControlNameMap[qctrl.id] = qctrl.name;
                        m_ControlIdToFileDescriptorMap[qctrl.id] = fileDescriptor;
                        emit SendListIntDataToEnumerationWidget(id, value, list, name, unit, bIsReadOnly);
                    }
                }
            }
            qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }

        if (0 == cidCount)
        {
            LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s returned error, no controls can be enumerated.", m_FileDescriptorToNameMap[fileDescriptor].c_str());
        }
        else
        {
            LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s: NumControls=%d", m_FileDescriptorToNameMap[fileDescriptor].c_str(), cidCount);
            result = 0;
        }
    }
    return result;
}

template<typename T> T        getExtCtrlValue          (const v4l2_ext_control& extCtrl);
template<>           int32_t  getExtCtrlValue<int32_t> (const v4l2_ext_control& extCtrl) {return extCtrl.value;}
template<>           uint32_t getExtCtrlValue<uint32_t>(const v4l2_ext_control& extCtrl) {return extCtrl.value;}
template<>           int64_t  getExtCtrlValue<int64_t> (const v4l2_ext_control& extCtrl) {return extCtrl.value64;}
template<>           uint64_t getExtCtrlValue<uint64_t>(const v4l2_ext_control& extCtrl) {return extCtrl.value64;}

template<typename T>
int Camera::ReadExtControl(int fileDescriptor, T &value, uint32_t controlID, const char *functionName, const char *controlName, uint32_t controlClass)
{
    QMutexLocker locker(&m_ReadExtControlMutex);
    int result = -1;
    v4l2_queryctrl ctrl;

    CLEAR(ctrl);
    ctrl.id = controlID;

    if (iohelper::xioctl(fileDescriptor, VIDIOC_QUERYCTRL, &ctrl) >= 0)
    {
        v4l2_ext_controls extCtrls;
        v4l2_ext_control extCtrl;

        LOG_EX("Camera::ReadExtControl VIDIOC_QUERYCTRL %s function name: %s control name: %s OK, min=%d, max=%d, default=%d", m_FileDescriptorToNameMap[fileDescriptor].c_str(), functionName, controlName, ctrl.minimum, ctrl.maximum, ctrl.default_value);

        CLEAR(extCtrls);
        CLEAR(extCtrl);
        extCtrl.id = controlID;

        extCtrls.controls = &extCtrl;
        extCtrls.count = 1;
        extCtrls.ctrl_class = controlClass;

        if (-1 != iohelper::xioctl(fileDescriptor, VIDIOC_G_EXT_CTRLS, &extCtrls))
        {
            LOG_EX("Camera::ReadExtControl VIDIOC_G_EXT_CTRLS %s function name: %s control name: %s OK =%d", m_FileDescriptorToNameMap[fileDescriptor].c_str(), functionName, controlName, extCtrl.value);

            value = getExtCtrlValue<T>(extCtrl);

            result = 0;
        }
        else
        {
            LOG_EX("Camera::ReadExtControl VIDIOC_G_CTRL %s function name: %s control name: %s failed errno=%d=%s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());

            result = -2;
        }
    }
    else
    {
        LOG_EX("Camera::ReadExtControl VIDIOC_QUERYCTRL %s function name: %s control name: %s failed errno=%d=%s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());

        result = -2;
    }

    return result;
}

template<typename T>
int Camera::ReadExtControl(T &value, uint32_t controlID, const char *functionName, const char* controlName, uint32_t controlClass)
{
    LOG_EX("Camera::ReadExtControl %s control %s", m_FileDescriptorToNameMap[m_ControlIdToFileDescriptorMap[controlID]].c_str(), m_ControlIdToControlNameMap[controlID].c_str());
    return ReadExtControl(m_ControlIdToFileDescriptorMap[controlID], value, controlID, functionName, controlName, controlClass);
}

template<typename T> void setExtCtrlValue                         (v4l2_ext_control& extCtrl, const T& value);
template<>           void setExtCtrlValue<int32_t>                (v4l2_ext_control& extCtrl, const int32_t& value)                 {extCtrl.value = value;}
template<>           void setExtCtrlValue<uint32_t>               (v4l2_ext_control& extCtrl, const uint32_t& value)                {extCtrl.value = value;}
template<>           void setExtCtrlValue<int64_t>                (v4l2_ext_control& extCtrl, const int64_t& value)                 {extCtrl.value64 = value;}
template<>           void setExtCtrlValue<uint64_t>               (v4l2_ext_control& extCtrl, const uint64_t& value)                {extCtrl.value64 = value;}
template<>           void setExtCtrlValue<v4l2_exposure_auto_type>(v4l2_ext_control& extCtrl, const v4l2_exposure_auto_type& value) {extCtrl.value = value;}
template<>           void setExtCtrlValue<bool>                   (v4l2_ext_control& extCtrl, const bool& value)                    {extCtrl.value = value;}

template<typename T>
int Camera::SetExtControl(T value, uint32_t controlID, const char *functionName, const char *controlName, uint32_t controlClass)
{
    int result = -1;
    v4l2_ext_controls extCtrls;
    v4l2_ext_control extCtrl;

    CLEAR(extCtrls);
    CLEAR(extCtrl);
    extCtrl.id = controlID;
    setExtCtrlValue<T>(extCtrl, value);

    extCtrls.controls = &extCtrl;
    extCtrls.count = 1;
    extCtrls.ctrl_class = controlClass;


    if (-1 != iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_TRY_EXT_CTRLS, &extCtrls))
    {
        if (-1 != iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_S_EXT_CTRLS, &extCtrls))
        {
            LOG_EX("Camera::SetExtControl VIDIOC_S_EXT_CTRLS %s function name: %s control name: %s (%s) to %d OK", m_FileDescriptorToNameMap[m_ControlIdToFileDescriptorMap[controlID]].c_str(), functionName, controlName, m_ControlIdToControlNameMap[controlID].c_str(), value);
            result = 0;
        }
        else
        {
            LOG_EX("Camera::SetExtControlVIDIOC_S_EXT_CTRLS %s function name: %s control name: %s (%s) failed errno=%d=%s", m_FileDescriptorToNameMap[m_ControlIdToFileDescriptorMap[controlID]].c_str(), functionName, controlName, m_ControlIdToControlNameMap[controlID].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
    }
    else
    {
        LOG_EX("Camera::SetExtControl VIDIOC_TRY_EXT_CTRLS %s function name: %s control name: %s (%s) failed errno=%d=%s", m_FileDescriptorToNameMap[m_ControlIdToFileDescriptorMap[controlID]].c_str(), functionName, controlName, m_ControlIdToControlNameMap[controlID].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

template<typename T> struct v4l2_ctrl_type_container;
template<>           struct v4l2_ctrl_type_container<int32_t>  {using type = v4l2_queryctrl;};
template<>           struct v4l2_ctrl_type_container<uint32_t> {using type = v4l2_queryctrl;};
template<>           struct v4l2_ctrl_type_container<int64_t>  {using type = v4l2_query_ext_ctrl;};
template<>           struct v4l2_ctrl_type_container<uint64_t> {using type = v4l2_query_ext_ctrl;};

template<typename T> constexpr int vidioc_queryctrl;
template<> constexpr int vidioc_queryctrl<int32_t> = VIDIOC_QUERYCTRL;
template<> constexpr int vidioc_queryctrl<uint32_t> = VIDIOC_QUERYCTRL;
template<> constexpr int vidioc_queryctrl<int64_t> = VIDIOC_QUERY_EXT_CTRL;
template<> constexpr int vidioc_queryctrl<uint64_t> = VIDIOC_QUERY_EXT_CTRL;

template<typename T>
int Camera::ReadMinMax(T &min, T &max, uint32_t controlID, const char *functionName, const char *controlName)
{
    int result = -1;
    typename v4l2_ctrl_type_container<T>::type ctrl;

    CLEAR(ctrl);
    ctrl.id = controlID;

    if (iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], vidioc_queryctrl<T>, &ctrl) >= 0)
    {
        LOG_EX("Camera::ReadMinMax VIDIOC_QUERYCTRL %s function name: %s control name: %s (%s) OK, min=%d, max=%d, default=%d", m_FileDescriptorToNameMap[m_ControlIdToFileDescriptorMap[controlID]].c_str(), functionName, controlName, m_ControlIdToControlNameMap[controlID].c_str(), ctrl.minimum, ctrl.maximum, ctrl.default_value);
        min = ctrl.minimum;
        max = ctrl.maximum;
    }
    else
    {
        LOG_EX("Camera::ReadMinMax VIDIOC_QUERYCTRL %s function name: %s control name: %s (%s) failed errno=%d=%s", m_FileDescriptorToNameMap[m_ControlIdToFileDescriptorMap[controlID]].c_str(), functionName, controlName, m_ControlIdToControlNameMap[controlID].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        result = -2;
    }

    return result;
}

int Camera::ReadStep(int32_t &step, uint32_t controlID, const char *functionName, const char *controlName)
{
    int result = 0;
    v4l2_queryctrl ctrl;

    CLEAR(ctrl);
    ctrl.id = controlID;
    if (iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_QUERYCTRL, &ctrl) >= 0)
    {
        LOG_EX("Camera::ReadStep VIDIOC_QUERYCTRL %s function name: %s control name: %s (%s) OK, step=%d", m_FileDescriptorToNameMap[m_ControlIdToFileDescriptorMap[controlID]].c_str(), functionName, controlName, m_ControlIdToControlNameMap[controlID].c_str(), ctrl.step);
        step = ctrl.step;
    }
    else
    {
        LOG_EX("Camera::ReadStep VIDIOC_QUERYCTRL %s function name: %s control name: %s (%s) failed errno=%d=%s", m_FileDescriptorToNameMap[m_ControlIdToFileDescriptorMap[controlID]].c_str(), functionName, controlName, m_ControlIdToControlNameMap[controlID].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        result = -2;
    }

    return result;
}


int Camera::ReadMinMaxExposure(int64_t &min, int64_t &max)
{
    return ReadMinMax(min, max, V4L2_CID_EXPOSURE, "ReadExposureMinMax", "V4L2_CID_EXPOSURE");
}

int Camera::ReadAutoExposure(bool &flag)
{
    int result = 0;
    uint32_t value = 0;
    result = ReadExtControl(value, V4L2_CID_EXPOSURE_AUTO, "ReadAutoExposure", "V4L2_CID_EXPOSURE_AUTO", V4L2_CTRL_CLASS_CAMERA);
    flag = (value == V4L2_EXPOSURE_AUTO) ? true : false;

    return result;
}

int Camera::SetAutoExposure(bool autoexposure)
{
	uint32_t value = autoexposure ? V4L2_EXPOSURE_AUTO : V4L2_EXPOSURE_MANUAL;

	emit SendListDataToEnumerationWidget(V4L2_CID_EXPOSURE_AUTO, value, QList<QString>(), "", "", false);

    return SetExtControl(value, V4L2_CID_EXPOSURE_AUTO, "SetAutoExposure", "V4L2_CID_EXPOSURE_AUTO", V4L2_CTRL_CLASS_CAMERA);
}

//////////////////// Controls ////////////////////////

int Camera::ReadGain(int64_t &value)
{
    return ReadExtControl(value, V4L2_CID_GAIN, "ReadGain", "V4L2_CID_GAIN", V4L2_CTRL_CLASS_USER);
}

int Camera::SetGain(int64_t value)
{
    return SetExtControl(value, V4L2_CID_GAIN, "SetGain", "V4L2_CID_GAIN", V4L2_CTRL_CLASS_USER);
}

int Camera::ReadMinMaxGain(int64_t &min, int64_t &max)
{
    return ReadMinMax(min, max, V4L2_CID_GAIN, "ReadMinMaxGain", "V4L2_CID_GAIN");
}

int Camera::ReadAutoGain(bool &flag)
{
    int result = 0;
    uint32_t value = 0;

    result = ReadExtControl(value, V4L2_CID_AUTOGAIN, "ReadAutoGain", "V4L2_CID_AUTOGAIN", V4L2_CTRL_CLASS_USER);

    flag = (value) ? true : false;

    return result;
}

int Camera::SetAutoGain(bool value)
{
	emit SendBoolDataToEnumerationWidget(V4L2_CID_AUTOGAIN, value, "", "", false);

    return SetExtControl(value, V4L2_CID_AUTOGAIN, "SetAutoGain", "V4L2_CID_AUTOGAIN", V4L2_CTRL_CLASS_USER);
}

int Camera::ReadExposure(int64_t &value)
{
    return ReadExtControl(value, V4L2_CID_EXPOSURE, "ReadExposure", "V4L2_CID_EXPOSURE", V4L2_CTRL_CLASS_USER);
}

int Camera::SetExposure(int64_t value)
{
    return SetExtControl(value, V4L2_CID_EXPOSURE, "SetExposure", "V4L2_CID_EXPOSURE", V4L2_CTRL_CLASS_USER);
}

int Camera::ReadGamma(int32_t &value)
{
    return ReadExtControl(value, V4L2_CID_GAMMA, "ReadGamma", "V4L2_CID_GAMMA", V4L2_CTRL_CLASS_USER);
}

int Camera::SetGamma(int32_t value)
{
    return SetExtControl(value, V4L2_CID_GAMMA, "SetGamma", "V4L2_CID_GAMMA", V4L2_CTRL_CLASS_USER);
}

int Camera::ReadMinMaxGamma(int64_t &min, int64_t &max)
{
    return ReadMinMax(min, max, V4L2_CID_GAMMA, "ReadMinMaxGamma", "V4L2_CID_GAMMA");
}

int Camera::ReadReverseX(int32_t &value)
{
    return ReadExtControl(value, V4L2_CID_HFLIP, "ReadReverseX", "V4L2_CID_HFLIP", V4L2_CTRL_CLASS_USER);
}

int Camera::SetReverseX(int32_t value)
{
    return SetExtControl(value, V4L2_CID_HFLIP, "SetReverseX", "V4L2_CID_HFLIP", V4L2_CTRL_CLASS_USER);
}

int Camera::ReadReverseY(int32_t &value)
{
    return ReadExtControl(value, V4L2_CID_VFLIP, "ReadReverseY", "V4L2_CID_VFLIP", V4L2_CTRL_CLASS_USER);
}

int Camera::SetReverseY(int32_t value)
{
    return SetExtControl(value, V4L2_CID_VFLIP, "SetReverseY", "V4L2_CID_VFLIP", V4L2_CTRL_CLASS_USER);
}

int Camera::ReadBrightness(int32_t &value)
{
    return ReadExtControl(value, V4L2_CID_BRIGHTNESS, "ReadBrightness", "V4L2_CID_BRIGHTNESS", V4L2_CTRL_CLASS_USER);
}

int Camera::SetBrightness(int32_t value)
{
    return SetExtControl(value, V4L2_CID_BRIGHTNESS, "SetBrightness", "V4L2_CID_BRIGHTNESS", V4L2_CTRL_CLASS_USER);
}

int Camera::ReadMinMaxBrightness(int32_t &min, int32_t &max)
{
    return ReadMinMax(min, max, V4L2_CID_BRIGHTNESS, "ReadMinMaxBrightness", "V4L2_CID_BRIGHTNESS");
}

bool Camera::IsAutoWhiteBalanceSupported()
{
    v4l2_queryctrl qctrl;

    CLEAR(qctrl);
    qctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;

    if (iohelper::xioctl(m_ControlIdToFileDescriptorMap[V4L2_CID_AUTO_WHITE_BALANCE], VIDIOC_QUERYCTRL, &qctrl) == 0)
    {
        return !(qctrl.flags & V4L2_CTRL_FLAG_DISABLED);
    }
    else
    {
        return false;
    }
}

int Camera::SetAutoWhiteBalance(bool flag)
{

	emit SendBoolDataToEnumerationWidget(V4L2_CID_AUTO_WHITE_BALANCE, flag,  "", "", false);

    if (flag)
    {

        return SetExtControl(flag, V4L2_CID_AUTO_WHITE_BALANCE, "SetContinousWhiteBalance on", "V4L2_CID_AUTO_WHITE_BALANCE", V4L2_CTRL_CLASS_USER);
    }
    else
    {

        return SetExtControl(flag, V4L2_CID_AUTO_WHITE_BALANCE, "SetContinousWhiteBalance off", "V4L2_CID_AUTO_WHITE_BALANCE", V4L2_CTRL_CLASS_USER);
    }
}

int Camera::ReadAutoWhiteBalance(bool &flag)
{
    int result = 0;
    uint32_t value = 0;
    result = ReadExtControl(value, V4L2_CID_AUTO_WHITE_BALANCE, "ReadAutoWhiteBalance", "V4L2_CID_AUTO_WHITE_BALANCE", V4L2_CTRL_CLASS_USER);
    flag = (value == V4L2_WHITE_BALANCE_AUTO) ? true : false;



    return result;
}

void Camera::PrepareFrameRate()
{
    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        v4l2_streamparm parm;

        CLEAR(parm);
        parm.type = (fileDescriptor == m_DeviceFileDescriptor ? m_DeviceBufferType : m_SubDeviceBufferTypes[fileDescriptor]);

        if (iohelper::xioctl(fileDescriptor, VIDIOC_G_PARM, &parm) >= 0 && parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
        {
            m_FrameRateDeviceFileDescriptor = fileDescriptor;
            LOG_EX("Camera::PrepareFrameRate VIDIOC_G_CROP %s", m_FileDescriptorToNameMap[m_FrameRateDeviceFileDescriptor].c_str());
        }
    }

    if(m_FrameRateDeviceFileDescriptor < 0)
    {
        LOG_EX("Camera::PrepareFrameRate no device for frame rate");
    }
}

void Camera::PrepareCrop()
{
    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        v4l2_crop crop;

        CLEAR(crop);
        crop.type = m_DeviceBufferType;

        if (iohelper::xioctl(fileDescriptor, VIDIOC_G_CROP, &crop) >= 0)
        {
            m_CropDeviceFileDescriptor = fileDescriptor;
            LOG_EX("Camera::PrepareCrop VIDIOC_G_CROP %s", m_FileDescriptorToNameMap[m_CropDeviceFileDescriptor].c_str());
        }
    }

    if(m_CropDeviceFileDescriptor < 0)
    {
        LOG_EX("Camera::PrepareCrop no device for cropping");
    }
}


bool Camera::UsesSubdevices() {
    return !m_SubDeviceFileDescriptors.empty();
}


////////////////// GetDeviceChar ///////////////////


std::string Camera::GetDeviceChar(uint32_t controlId)
{
    std::string deviceChar = "unknown";
    if(m_ControlIdToFileDescriptorMap.count(controlId))
    {
        deviceChar = GetDeviceCharFromFileDescriptor(m_ControlIdToFileDescriptorMap[controlId]);
    }
    return deviceChar;
}

std::string Camera::GetDeviceCharFromFileDescriptor(int fileDescriptor)
{
    std::string deviceChar = "no device";
    if(m_FileDescriptorToNameMap.count(fileDescriptor))
    {
        const std::string deviceFileName = m_FileDescriptorToNameMap[fileDescriptor];
        if(deviceFileName.find("subdev") != std::string::npos)
        {
            deviceChar = "s";
        }
        else if(deviceFileName.find("video") != std::string::npos)
        {
            deviceChar = "d";
        }
        else
        {
            deviceChar = deviceFileName;
        }
    }
    return deviceChar;
}

std::string Camera::GetGainDeviceChar()
{
    return GetDeviceChar(V4L2_CID_GAIN);
}

std::string Camera::GetAutoGainDeviceChar()
{
    return GetDeviceChar(V4L2_CID_AUTOGAIN);
}

std::string Camera::GetExposureDeviceChar()
{
    return GetDeviceChar(V4L2_CID_EXPOSURE);
}

std::string Camera::GetExposureAbsDeviceChar()
{
    return GetDeviceChar(V4L2_CID_EXPOSURE_ABSOLUTE);
}

std::string Camera::GetExposureAutoDeviceChar()
{
    return GetDeviceChar(V4L2_CID_EXPOSURE_AUTO);
}

std::string Camera::GetGammaDeviceChar()
{
    return GetDeviceChar(V4L2_CID_GAMMA);
}

std::string Camera::GetReverseXDeviceChar()
{
    return GetDeviceChar(V4L2_CID_HFLIP);
}

std::string Camera::GetReverseYDeviceChar()
{
    return GetDeviceChar(V4L2_CID_VFLIP);
}

std::string Camera::GetBrightnessDeviceChar()
{
    return GetDeviceChar(V4L2_CID_BRIGHTNESS);
}

std::string Camera::GetAutoWhiteBalanceDeviceChar()
{
    return GetDeviceChar(V4L2_CID_AUTO_WHITE_BALANCE);
}

std::string Camera::GetFrameRateDeviceChar()
{
    return GetDeviceCharFromFileDescriptor(m_FrameRateDeviceFileDescriptor);
}

std::string Camera::GetCropDeviceChar()
{
    return GetDeviceCharFromFileDescriptor(m_CropDeviceFileDescriptor);
}

std::string Camera::GetPixelFormatDeviceChar()
{
    return GetDeviceCharFromFileDescriptor(m_DeviceFileDescriptor);
}

std::string Camera::GetWidthDeviceChar()
{
    return GetDeviceCharFromFileDescriptor(m_DeviceFileDescriptor);
}

std::string Camera::GetHeightDeviceChar()
{
    return GetDeviceCharFromFileDescriptor(m_DeviceFileDescriptor);
}

////////////////// Parameter ///////////////////

int Camera::ReadFrameRate(uint32_t &numerator, uint32_t &denominator, uint32_t width, uint32_t height, uint32_t pixelFormat)
{
    int result = -1;

    v4l2_streamparm parm;

    CLEAR(parm);
    parm.type = (m_FrameRateDeviceFileDescriptor == m_DeviceFileDescriptor ? m_DeviceBufferType : m_SubDeviceBufferTypes[m_FrameRateDeviceFileDescriptor]);

    if (iohelper::xioctl(m_FrameRateDeviceFileDescriptor, VIDIOC_G_PARM, &parm) >= 0 && parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
    {
        v4l2_frmivalenum frmival;
        CLEAR(frmival);

        frmival.index = 0;
        frmival.pixel_format = pixelFormat;
        frmival.width = width;
        frmival.height = height;
        while (iohelper::xioctl(m_FrameRateDeviceFileDescriptor, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0)
        {
            frmival.index++;
            LOG_EX("Camera::ReadFrameRate type=%d", frmival.type);
        }
        if (frmival.index == 0)
        {
            LOG_EX("Camera::ReadFrameRate VIDIOC_ENUM_FRAMEINTERVALS %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_FrameRateDeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }

        v4l2_streamparm parm;
        CLEAR(parm);
        parm.type = m_DeviceBufferType;

        if (iohelper::xioctl(m_FrameRateDeviceFileDescriptor, VIDIOC_G_PARM, &parm) >= 0)
        {
            numerator = parm.parm.capture.timeperframe.numerator;
            denominator = parm.parm.capture.timeperframe.denominator;
            LOG_EX("Camera::ReadFrameRate VIDIOC_G_PARM %s %d/%dOK", m_FileDescriptorToNameMap[m_FrameRateDeviceFileDescriptor].c_str(), numerator, denominator);
        }
        else
        {
            LOG_EX("Camera::ReadFrameRate VIDIOC_G_PARM %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_FrameRateDeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
    }
    else
    {
        LOG_EX("Camera::ReadFrameRate VIDIOC_G_PARM %s failed (or V4L2_CAP_TIMEPERFRAME not in cap) errno=%d=%s", m_FileDescriptorToNameMap[m_FrameRateDeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        result = -2;
    }

    return result;
}

int Camera::SetFrameRate(uint32_t numerator, uint32_t denominator)
{
    int result = -1;

    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        v4l2_streamparm parm;

        CLEAR(parm);
        parm.type = (m_FrameRateDeviceFileDescriptor == m_DeviceFileDescriptor ? m_DeviceBufferType : m_SubDeviceBufferTypes[m_FrameRateDeviceFileDescriptor]);
        if (denominator != 0)
        {
            if (iohelper::xioctl(fileDescriptor, VIDIOC_G_PARM, &parm) >= 0)
            {
                parm.parm.capture.timeperframe.numerator = numerator;
                parm.parm.capture.timeperframe.denominator = denominator;
                if (-1 != iohelper::xioctl(fileDescriptor, VIDIOC_S_PARM, &parm))
                {
                    LOG_EX("Camera::SetFrameRate VIDIOC_S_PARM %s to %d/%d (%.2f) OK", m_FileDescriptorToNameMap[fileDescriptor].c_str(), numerator, denominator, numerator / denominator);
                    result = 0;
                }
                else
                {
                    LOG_EX("Camera::SetFrameRate VIDIOC_S_PARM %s failed errno=%d=%s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
                }
            }
        }
    }

    return result;
}

int Camera::ReadCrop(int32_t &xOffset, int32_t &yOffset, uint32_t &width, uint32_t &height)
{
    int result = -2;

    v4l2_crop crop;

    CLEAR(crop);
    crop.type = (m_CropDeviceFileDescriptor == m_DeviceFileDescriptor ? m_DeviceBufferType : m_SubDeviceBufferTypes[m_CropDeviceFileDescriptor]);

    if (iohelper::xioctl(m_CropDeviceFileDescriptor, VIDIOC_G_CROP, &crop) >= 0)
    {
        xOffset = crop.c.left;
        yOffset = crop.c.top;
        width = crop.c.width;
        height = crop.c.height;
        LOG_EX("Camera::ReadCrop VIDIOC_G_CROP %s x=%d, y=%d, w=%d, h=%d OK", m_FileDescriptorToNameMap[m_CropDeviceFileDescriptor].c_str(), xOffset, yOffset, width, height);

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadCrop VIDIOC_G_CROP %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_CropDeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::SetCrop(int32_t xOffset, int32_t yOffset, uint32_t width, uint32_t height)
{
    int result = -1;

    v4l2_crop crop;

    CLEAR(crop);
    crop.type = (m_CropDeviceFileDescriptor == m_DeviceFileDescriptor ? m_DeviceBufferType : m_SubDeviceBufferTypes[m_CropDeviceFileDescriptor]);
    crop.c.left = xOffset;
    crop.c.top = yOffset;
    crop.c.width = width;
    crop.c.height = height;

    if (iohelper::xioctl(m_CropDeviceFileDescriptor, VIDIOC_S_CROP, &crop) >= 0)
    {
        LOG_EX("Camera::SetCrop VIDIOC_S_CROP %s left=%d, top=%d, width=%d, height=%d OK", m_FileDescriptorToNameMap[m_CropDeviceFileDescriptor].c_str(), xOffset, yOffset, width, height);

        result = 0;
    }
    else
    {
        LOG_EX("Camera::SetCrop VIDIOC_S_CROP %s failed errno=%d=%s", m_FileDescriptorToNameMap[m_CropDeviceFileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

/*********************************************************************************************************/
// Frame buffer handling
/*********************************************************************************************************/

int Camera::CreateUserBuffer(uint32_t bufferCount, uint32_t bufferSize)
{
    int ret = -1;

    m_pFrameObserver->setFileDescriptor(m_DeviceFileDescriptor);
    m_pFrameObserver->setBufferType(m_DeviceBufferType);

    ret = m_pFrameObserver->CreateAllUserBuffer(bufferCount, bufferSize);

    return ret;
}

int Camera::QueueAllUserBuffer()
{
    int result = -1;

    result = m_pFrameObserver->QueueAllUserBuffer();

    return result;
}

int Camera::QueueSingleUserBuffer(const int index)
{
    int result = 0;

    result = m_pFrameObserver->QueueSingleUserBuffer(index);

    return result;
}

int Camera::DeleteUserBuffer()
{
    int result = 0;

    result = m_pFrameObserver->DeleteAllUserBuffer();

    return result;
}

/*********************************************************************************************************/
// Info
/*********************************************************************************************************/

int Camera::GetCameraDriverName(std::string &strText)
{
    int result = -1;
    std::string info;
    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        v4l2_capability cap;

        // query device capabilities
        if (-1 == iohelper::xioctl(fileDescriptor, VIDIOC_QUERYCAP, &cap))
        {
            LOG_EX("Camera::GetCameraDriverName VIDIOC_QUERYCAP %s is no V4L2 device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
        }
        else
        {
            if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
            {
                LOG_EX("Camera::GetCameraDriverName %s is no video capture device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
            }
            else
            {
                if(((char* )cap.driver)[0] == '\0')
                {
                    LOG_EX("Camera::GetCameraDriverName VIDIOC_QUERYCAP %s no driver name\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
                }
                else
                {
                    LOG_EX("Camera::GetCameraDriverName VIDIOC_QUERYCAP %s driver name=%s\n", m_FileDescriptorToNameMap[fileDescriptor].c_str(), (char* )cap.driver);
                    if (info.empty())
                    {
                        info = std::string((char*) cap.driver) + " (" + m_FileDescriptorToNameMap[fileDescriptor] + ")";
                    }
                    else
                    {
                        info += std::string(",<br>") + std::string((char*) cap.driver) + " (" + m_FileDescriptorToNameMap[fileDescriptor] + ")";
                    }
                    result = 0;
                }
            }
        }
    }

    strText = info;

    return result;
}

int Camera::GetCameraDeviceName(std::string &strText)
{
    int result = -1;
    std::string info;
    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        v4l2_capability cap;

        // query device capabilities
        if (-1 == iohelper::xioctl(fileDescriptor, VIDIOC_QUERYCAP, &cap))
        {
            LOG_EX("Camera::GetCameraDeviceName VIDIOC_QUERYCAP %s is no V4L2 device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
        }
        else
        {
            if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
            {
                LOG_EX("Camera::GetCameraDeviceName %s is no video capture device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
            }
            else
            {
                if(((char* )cap.card)[0] == '\0')
                {
                    LOG_EX("Camera::GetCameraDeviceName VIDIOC_QUERYCAP %s no device name\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
                }
                else
                {
                    LOG_EX("Camera::GetCameraDeviceName VIDIOC_QUERYCAP %s device name=%s\n", m_FileDescriptorToNameMap[fileDescriptor].c_str(), (char* )cap.card);
                    if (info.empty())
                    {
                        info = std::string((char*) cap.card) + " (" + m_FileDescriptorToNameMap[fileDescriptor] + ")";
                    }
                    else
                    {
                        info += std::string(",<br>") + std::string((char*) cap.card) + " (" + m_FileDescriptorToNameMap[fileDescriptor] + ")";
                    }
                    result = 0;
                }
            }
        }
    }

    strText = info;

    return result;
}

int Camera::GetCameraBusInfo(std::string &strText)
{
    int result = -1;
    std::string info;
    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        v4l2_capability cap;

        // query device capabilities
        if (-1 == iohelper::xioctl(fileDescriptor, VIDIOC_QUERYCAP, &cap))
        {
            LOG_EX("Camera::GetCameraBusInfo VIDIOC_QUERYCAP %s is no V4L2 device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
        }
        else
        {
            if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
            {
                LOG_EX("Camera::GetCameraBusInfo %s is no video capture device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
            }
            else
            {
                if(((char* )cap.bus_info)[0] == '\0')
                {
                    LOG_EX("Camera::GetCameraBusInfo VIDIOC_QUERYCAP %s no bus info\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
                }
                else
                {
                    LOG_EX("Camera::GetCameraBusInfo VIDIOC_QUERYCAP %s bus info=%s\n", m_FileDescriptorToNameMap[fileDescriptor].c_str(), (char* )cap.bus_info);
                    if (info.empty())
                    {
                        info = std::string((char*) cap.bus_info) + " (" + m_FileDescriptorToNameMap[fileDescriptor] + ")";
                    }
                    else
                    {
                        info += std::string(",<br>") + std::string((char*) cap.bus_info) + " (" + m_FileDescriptorToNameMap[fileDescriptor] + ")";
                    }
                    result = 0;
                }
            }
        }
    }

    strText = info;

    return result;
}

int Camera::GetCameraDriverVersion(std::string &strText)
{
    int result = -1;
    std::string info;
    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        v4l2_capability cap;

        // query device capabilities
        if (-1 == iohelper::xioctl(fileDescriptor, VIDIOC_QUERYCAP, &cap))
        {
            LOG_EX("Camera::GetCameraDriverVersion VIDIOC_QUERYCAP %s is no V4L2 device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
        }
        else
        {
            if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
            {
                LOG_EX("Camera::GetCameraDriverVersion %s is no video capture device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
            }
            else
            {
                if(((char* )cap.card)[0] == '\0')
                {
                    LOG_EX("Camera::GetCameraDriverVersion VIDIOC_QUERYCAP %s no driver version info\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
                }
                else
                {
                    std::string cameraDriverInfo = (char*)cap.card;

                    LOG_EX("Camera::GetCameraDriverVersion VIDIOC_QUERYCAP %s driver version info string=%s\n", m_FileDescriptorToNameMap[fileDescriptor].c_str(), cameraDriverInfo.c_str());

                    QString name = QString::fromStdString(cameraDriverInfo);
                    QStringList list = name.split(" ");
                    std::string version = "unknown";

                    if (!list.isEmpty() && list.back().contains('-'))
                    {
                        QString part = list.back();
                        QString rightPart = part.mid(part.indexOf('-')+1);
                        QString leftPart = part.mid(0, part.indexOf('-'));

                        QString numbers;
                        QString letters;
                        for (QString::iterator it = rightPart.begin(); it != rightPart.end(); ++it)
                        {
                            if(it->isLetter())
                            {
                                int index = it - rightPart.begin();
                                numbers = rightPart.mid(0, index);
                                letters = rightPart.mid(index);
                                break;
                            }
                        }

                        int num = numbers.toInt();
                        QString parsedNumbers = QString("%1").arg(num, 3, 10,  QLatin1Char('0'));
                        rightPart = parsedNumbers + letters;
                        part = leftPart + '-' + rightPart;

                        QFile file(QString("/sys/bus/i2c/drivers/avt_csi2/%1/driver_version").arg(part));
                        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                        {
                            LOG_EX("Camera::GetCameraDriverVersion Couldn't get driver version for VIDIOC_QUERYCAP %s device name=%s, driver version file=%s\n", m_FileDescriptorToNameMap[fileDescriptor].c_str(), (char*)cap.card, file.fileName().toStdString().c_str());
                            QFile file_alt(QString("/sys/bus/i2c/drivers/avt3/%1/driver_version").arg(part));
                            if (!file_alt.open(QIODevice::ReadOnly | QIODevice::Text))
                            {
                                LOG_EX("Camera::GetCameraDriverVersion Couldn't get driver version for VIDIOC_QUERYCAP %s device name=%s, driver version file=%s\n", m_FileDescriptorToNameMap[fileDescriptor].c_str(), (char*)cap.card, file_alt.fileName().toStdString().c_str());
                                version = "unknown";
                            }
                            else
                            {
                                QByteArray line = file_alt.readLine();
                                version = line.toStdString();
                                LOG_EX("Camera::GetCameraDriverVersion got driver version for VIDIOC_QUERYCAP %s device name=%s, driver version file=%s\n", m_FileDescriptorToNameMap[fileDescriptor].c_str(), (char*)cap.card, file_alt.fileName().toStdString().c_str());
                            }
                        }
                        else
                        {
                            QByteArray line = file.readLine();
                            version = line.toStdString();
                            LOG_EX("Camera::GetCameraDriverVersion got driver version for VIDIOC_QUERYCAP %s device name=%s, driver version file=%s\n", m_FileDescriptorToNameMap[fileDescriptor].c_str(), (char*)cap.card, file.fileName().toStdString().c_str());
                        }
                    }
                    else
                    {
                        LOG_EX("Camera::GetCameraDriverVersion Couldn't get driver version for VIDIOC_QUERYCAP %s device name=%s\n", m_FileDescriptorToNameMap[fileDescriptor].c_str(), (char*)cap.card);
                        version = "unknown";
                    }

                    if (info.empty())
                    {
                        info = version + " (" + m_FileDescriptorToNameMap[fileDescriptor] + ")";
                    }
                    else
                    {
                        info += std::string(",<br>") + version + " (" + m_FileDescriptorToNameMap[fileDescriptor] + ")";
                    }
                    result = 0;
                }
            }
        }

    }

    strText = info;

    return result;
}

int Camera::GetCameraReadCapability(bool &flag)
{
    int result = -1;
    flag = false;

    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        std::stringstream tmp;
        v4l2_capability cap;

        // query device capabilities
        if (-1 == iohelper::xioctl(fileDescriptor, VIDIOC_QUERYCAP, &cap))
        {
            LOG_EX("Camera::GetCameraReadCapability %s is no V4L2 device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
        }
        else
        {
            if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
            {
                LOG_EX("Camera::GetCameraReadCapability %s is no video capture device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
            }
            else
            {
                tmp << "0x" << std::hex << cap.capabilities << std::endl << "    Read/Write = " << ((cap.capabilities & V4L2_CAP_READWRITE) ? "Yes" : "No") << std::endl << "    Streaming = " << ((cap.capabilities & V4L2_CAP_STREAMING) ? "Yes" : "No");
                LOG_EX("Camera::GetCameraReadCapability VIDIOC_QUERYCAP %s capabilities=%s\n", m_FileDescriptorToNameMap[fileDescriptor].c_str(), tmp.str().c_str());

                if(result == 0)
                {
                    LOG_EX("Camera::GetCameraReadCapability VIDIOC_QUERYCAP %s setting flag again", m_FileDescriptorToNameMap[fileDescriptor].c_str());
                }

                if (cap.capabilities & V4L2_CAP_READWRITE)
                {
                    flag = true;
                }
                else
                {
                    flag = false;
                }

                result = 0;
            }
        }
    }

    return result;
}

int Camera::GetCameraCapabilities(std::string &strText)
{
    int result = -1;
    std::string info;
    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        std::stringstream tmp;
        v4l2_capability cap;

        // query device capabilities
        if (-1 == iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_QUERYCAP, &cap))
        {
            LOG_EX("Camera::GetCameraCapabilities %s is no V4L2 device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
        }
        else
        {
            std::string driverName((char*) cap.driver);
            std::string avtDriverName = "avt";
            if (driverName.find(avtDriverName) != std::string::npos)
            {
                m_IsAvtCamera = true;
            }

            if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
            {
                LOG_EX("Camera::GetCameraCapabilities %s is no video capture device\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
            }
            else
            {
                tmp << "0x" << std::hex << cap.capabilities << std::endl << "    Read/Write = " << ((cap.capabilities & V4L2_CAP_READWRITE) ? "Yes" : "No") << std::endl << "    Streaming = " << ((cap.capabilities & V4L2_CAP_STREAMING) ? "Yes" : "No");
                LOG_EX("Camera::GetCameraCapabilities VIDIOC_QUERYCAP %s capabilities=%s\n", m_FileDescriptorToNameMap[fileDescriptor].c_str(), tmp.str().c_str());
                if (info.empty())
                {
                    info = tmp.str() + " (" + m_FileDescriptorToNameMap[fileDescriptor] + ") ";
                }
                else
                {
                    info += std::string(",<br>") + tmp.str() + " (" + m_FileDescriptorToNameMap[fileDescriptor] + ") ";
                }
                result = 0;
            }
        }
    }

    strText = info;

    return result;
}

/*********************************************************************************************************/
// Commands
/*********************************************************************************************************/

void Camera::SwitchFrameTransfer2GUI(bool showFrames)
{
    if (m_pFrameObserver != 0)
        m_pFrameObserver->SwitchFrameTransfer2GUI(showFrames);

    m_ShowFrames = showFrames;
}

/*********************************************************************************************************/
// Tools
/*********************************************************************************************************/

size_t Camera::fsize(const char *filename)
{
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

std::string Camera::getAvtDeviceFirmwareVersion()
{
    std::string result = "";

    if(m_pFrameObserver)
    {
        // dummy call to set m_isAvtCamera
        std::string dummy;
        GetCameraCapabilities(dummy);

        if(m_IsAvtCamera)
        {
            const int CCI_BCRM_REG = 0x0014;
            const int BCRM_DEV_FW_VERSION = 0x0010;

            uint16_t nBCRMAddress = 0;

            int res = ReadRegister(CCI_BCRM_REG, &nBCRMAddress, sizeof(nBCRMAddress), true);

            if (res >= 0)
            {
                uint64_t deviceFirmwareVersion = 0;

                res = ReadRegister((__u32)nBCRMAddress + BCRM_DEV_FW_VERSION, &deviceFirmwareVersion, sizeof(deviceFirmwareVersion), true);

                if (res >= 0)
                {
                    uint8_t specialVersion = (deviceFirmwareVersion & 0xFF);
                    uint8_t majorVersion = (deviceFirmwareVersion >> 8) & 0xFF;
                    uint16_t minorVersion = (deviceFirmwareVersion >> 16) & 0xFFFF;
                    uint32_t patchVersion = (deviceFirmwareVersion >> 32) & 0xFFFFFFFF;

                    std::stringstream ss;
                    ss << (unsigned)specialVersion << "." << (unsigned)majorVersion << "." << (unsigned)minorVersion << "." << std::hex << std::setw(8) << std::setfill('0') << (unsigned)patchVersion;
                    result = ss.str();
                }
            }
        }
    }

    return result;
}


std::string Camera::getAvtDeviceSerialNumber()
{
    std::string result = "";

    if(m_pFrameObserver)
    {
        // dummy call to set m_isAvtCamera
        std::string dummy;
        GetCameraCapabilities(dummy);

        if(m_IsAvtCamera)
        {
            const int DEVICE_SERIAL_NUMBER = 0x0198;
            char pBuf[64];

            int res = ReadRegister(DEVICE_SERIAL_NUMBER, &pBuf, sizeof(pBuf), false);        // String Reg! NO endianess conversion!

            if (res >= 0)
            {
                std::stringstream ss;

                // buffer may contain trailing null terminators
                for(unsigned int i=0; i<sizeof(pBuf); i++)
                {
                    char c = pBuf[i];
                    if(c != 0)
                        ss << pBuf[i];
                }

                result = ss.str();
            }
        }
    }

    return result;
}

bool Camera::getDriverStreamStat(uint64_t &FramesCount, uint64_t &PacketCRCError, uint64_t &FramesUnderrun, uint64_t &FramesIncomplete, double &CurrentFrameRate)
{
    bool result = false;

    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        if(m_pFrameObserver)
        {
            // dummy call to set m_isAvtCamera
            std::string dummy;
            GetCameraCapabilities(dummy);

            if(m_IsAvtCamera)
            {
                v4l2_stats_t stream_stats;
                CLEAR(stream_stats);
                if (ioctl(fileDescriptor, VIDIOC_STREAMSTAT, &stream_stats) >= 0)
                {
                    FramesCount = stream_stats.frames_count;
                    PacketCRCError = stream_stats.packet_crc_error;
                    FramesUnderrun = stream_stats.frames_underrun;
                    FramesIncomplete = stream_stats.frames_incomplete;
                    CurrentFrameRate = (stream_stats.current_frame_interval > 0) ? (double)stream_stats.current_frame_count / ((double)stream_stats.current_frame_interval / 1000000.0)
                                                                                : 0;
                    result = true;
                    LOG_EX("Camera::GetCameraCapabilities VIDIOC_STREAMSTAT %s OK\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
                }
                else
                {
                    LOG_EX("Camera::GetCameraCapabilities VIDIOC_STREAMSTAT %s not OK\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
                }
            }
        }
    }

    return result;
}

void Camera::reverseBytes(void* start, int size)
{
    char *istart = (char*)start, *iend = istart + size;
    std::reverse(istart, iend);
}


int Camera::ReadRegister(uint16_t nRegAddr, void* pBuffer, uint32_t nBufferSize, bool bConvertEndianess)
{
    int iRet = -1;
    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        v4l2_i2c i2c_reg;
        i2c_reg.register_address = (__u32)nRegAddr;
        i2c_reg.register_size = (__u32)2;
        i2c_reg.num_bytes = (__u32)nBufferSize;
        i2c_reg.ptr_buffer = (char*)pBuffer;

        int res = iohelper::xioctl(fileDescriptor, VIDIOC_R_I2C, &i2c_reg);

        if (res >= 0)
        {
            iRet = 0;

            // Values in BCM register are Big Endian -> swap bytes
            if(bConvertEndianess && (QSysInfo::ByteOrder == QSysInfo::LittleEndian))
            {
                reverseBytes(pBuffer, nBufferSize);
            }
            LOG_EX("Camera::ReadRegister VIDIOC_R_I2C %s OK\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
        }
        else
        {
            LOG_EX("Camera::ReadRegister VIDIOC_R_I2C %s not OK\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
        }
    }

    return iRet;
}

int Camera::WriteRegister(uint16_t nRegAddr, void* pBuffer, uint32_t nBufferSize, bool bConvertEndianess)
{
    int iRet = -1;
    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        // Values in BCM register are Big Endian -> swap bytes
        if(bConvertEndianess && (QSysInfo::ByteOrder == QSysInfo::LittleEndian))
        {
            reverseBytes(pBuffer, nBufferSize);
        }

        v4l2_i2c i2c_reg;
        i2c_reg.register_address = (__u32)nRegAddr;
        i2c_reg.register_size = (__u32)2;
        i2c_reg.num_bytes = (__u32)nBufferSize;
        i2c_reg.ptr_buffer = (char*)pBuffer;

        int res = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_W_I2C, &i2c_reg);

        if (res >= 0)
        {
            iRet = 0;
            LOG_EX("Camera::ReadRegister VIDIOC_R_I2C %s OK\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
        }
        else
        {
            LOG_EX("Camera::ReadRegister VIDIOC_R_I2C %s not OK\n", m_FileDescriptorToNameMap[fileDescriptor].c_str());
        }
    }

    return iRet;
}

void Camera::PassGainValue()
{
    int32_t value = 0;
    ReadExtControl(value, V4L2_CID_GAIN, "ReadGain", "V4L2_CID_GAIN", V4L2_CTRL_CLASS_USER);
    emit PassAutoGainValue(value);
}

void Camera::OnCtrlUpdate(int cid, v4l2_event_ctrl ctrl)
{
	switch (cid)
	{
		case V4L2_CID_GAIN:
			emit PassAutoGainValue(ctrl.value64);
			break;
		case V4L2_CID_EXPOSURE:
			emit PassAutoExposureValue(ctrl.value64);
			break;
		default:
			break;
	}

	switch (ctrl.type) {
		case V4L2_CTRL_TYPE_INTEGER:
			emit SendIntDataToEnumerationWidget(cid, 0, 0, ctrl.value, "", "", false);
			break;
		case V4L2_CTRL_TYPE_INTEGER_MENU:
			emit SendListIntDataToEnumerationWidget(cid, ctrl.value, QList<int64_t>(), "", "", false);
			break;
		case V4L2_CTRL_TYPE_MENU:
			emit SendListDataToEnumerationWidget(cid, ctrl.value, QList<QString>(), "", "", false);
			break;
		case V4L2_CTRL_TYPE_INTEGER64:
			emit SentInt64DataToEnumerationWidget(cid, 0, 0, ctrl.value64, "", "", false);
			break;
		case V4L2_CTRL_TYPE_BOOLEAN:
			emit SendBoolDataToEnumerationWidget(cid,ctrl.value,"","",false);
		default:
			break;
	}

	if (ctrl.changes & V4L2_EVENT_CTRL_CH_FLAGS)
	{
		if (ctrl.flags & V4L2_CTRL_FLAG_INACTIVE || ctrl.flags & V4L2_CTRL_FLAG_GRABBED)
		{
			emit SendControlStateChange(cid,false);
		}
		else
		{
			emit SendControlStateChange(cid,true);
		}
	}

}

void Camera::PassExposureValue()
{
    int64_t value = 0;
    ReadExtControl(value, V4L2_CID_EXPOSURE, "ReadExposure", "V4L2_CID_EXPOSURE", V4L2_CTRL_CLASS_USER);
    LOG_EX("Camera::PassExposureValue: exposure is now %d", value);
    emit PassAutoExposureValue(value);
}

void Camera::SetEnumerationControlValueIntList(int32_t id, int64_t val)
{
    int result = -1;

    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        v4l2_query_ext_ctrl qctrl;
        v4l2_ext_control extCtrl;
        v4l2_querymenu queryMenu;

        CLEAR(extCtrl);
        CLEAR(qctrl);
        CLEAR(queryMenu);

        extCtrl.id = id;
        qctrl.id = id;

        if (0 == iohelper::xioctl(fileDescriptor, VIDIOC_QUERY_EXT_CTRL, &qctrl))
        {
            queryMenu.id = qctrl.id;
            for (queryMenu.index = qctrl.minimum; queryMenu.index <= qctrl.maximum; queryMenu.index++)
            {
                if (0 == iohelper::xioctl(fileDescriptor, VIDIOC_QUERYMENU, &queryMenu))
                {
                    if (val == queryMenu.value)
                    {
                        LOG_EX("Camera::SetEnumerationControlValueIntList %s setting index for menu %s", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), queryMenu.name);
                        result = SetExtControl(queryMenu.index, id, "SetEnumerationControl", "V4L2_CTRL_TYPE_MENU", V4L2_CTRL_ID2CLASS(id));
                        if (result < 0)
                        {
                            LOG_EX("Camera::SetEnumerationControlValueIntList %s index for name %s cannot be set with V4L2_CTRL_CLASS_USER class", m_FileDescriptorToNameMap[m_DeviceFileDescriptor].c_str(), qctrl.name);
                        }
                        emit SendSignalToUpdateWidgets();
                        break;
                    }
                }
                else
                {
                    LOG_EX("Camera::SetEnumerationControlValueIntList VIDIOC_QUERYMENU %s failed errno=%d=%s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
                }
            }
        }
        else
        {
            LOG_EX("Camera::SetEnumerationControlValueIntList VIDIOC_QUERY_EXT_CTRL %s failed errno=%d=%s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
    }
}

void Camera::SetEnumerationControlValueList(int32_t id, const char *str)
{
    int result = -1;

    std::vector<int> allFileDescriptors = m_SubDeviceFileDescriptors;
    allFileDescriptors.push_back(m_DeviceFileDescriptor);

    for (const auto fileDescriptor : allFileDescriptors)
    {
        v4l2_query_ext_ctrl qctrl;
        v4l2_ext_control extCtrl;
        v4l2_querymenu queryMenu;

        CLEAR(extCtrl);
        CLEAR(qctrl);
        CLEAR(queryMenu);

        extCtrl.id = id;
        qctrl.id = id;

        if (0 == iohelper::xioctl(fileDescriptor, VIDIOC_QUERY_EXT_CTRL, &qctrl))
        {
            queryMenu.id = qctrl.id;
            for (queryMenu.index = qctrl.minimum; queryMenu.index <= qctrl.maximum; queryMenu.index++)
            {
                if (0 == iohelper::xioctl(fileDescriptor, VIDIOC_QUERYMENU, &queryMenu))
                {
                    if (strcmp((const char*) queryMenu.name, str) == 0)
                    {
                        LOG_EX("Camera::SetEnumerationControlValueList %s setting index for menu %s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), queryMenu.name);
                        result = SetExtControl(queryMenu.index, id, "SetEnumerationControl", "V4L2_CTRL_TYPE_MENU", V4L2_CTRL_ID2CLASS(id));
                        if (result < 0)
                        {
                            LOG_EX("Camera::SetEnumerationControlValueList %s index for name %s cannot be set with V4L2_CTRL_CLASS_USER class", m_FileDescriptorToNameMap[fileDescriptor].c_str(), queryMenu.name);
                        }
                        emit SendSignalToUpdateWidgets();
                        break;
                    }
                }
                else
                {
                    LOG_EX("Camera::SetEnumerationControlValueList VIDIOC_QUERYMENU %s failed errno=%d=%s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
                }
            }
        }
        else
        {
            LOG_EX("Camera::SetEnumerationControlValueList VIDIOC_QUERY_EXT_CTRL %s failed errno=%d=%s", m_FileDescriptorToNameMap[fileDescriptor].c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
    }
}

void Camera::SetEnumerationControlValue(int32_t id, int32_t val)
{
    int result = SetExtControl(val, id, "SetEnumerationControl", "V4L2_CTRL_TYPE_INTEGER32", V4L2_CTRL_ID2CLASS (id));
    if (result < 0)
    {
        LOG_EX("Enumeration control of type V4L2_CTRL_TYPE_INTEGER32 cannot be set with V4L2_CTRL_CLASS_USER class");
    }
    emit SendSignalToUpdateWidgets();
}

void Camera::SetEnumerationControlValue(int32_t id, int64_t val)
{
    int result = SetExtControl(val, id, "SetEnumerationControl", "V4L2_CTRL_TYPE_INTEGER64", V4L2_CTRL_ID2CLASS (id));
    if (result < 0)
    {
        LOG_EX("Enumeration control of type V4L2_CTRL_TYPE_INTEGER64 cannot be set with V4L2_CTRL_CLASS_USER class");
    }
    emit SendSignalToUpdateWidgets();
}

void Camera::SetEnumerationControlValue(int32_t id, bool val)
{
    int result = SetExtControl(static_cast<int32_t>(val), id, "SetEnumerationControl", "V4L2_CTRL_TYPE_BOOL", V4L2_CTRL_ID2CLASS (id));
    if (result < 0)
    {
        LOG_EX("Enumeration control of type V4L2_CTRL_TYPE_BOOL cannot be set with V4L2_CTRL_CLASS_USER class");
    }
    emit SendSignalToUpdateWidgets();
}

void Camera::SetEnumerationControlValue(int32_t id)
{
    int result = SetExtControl(0, id, "SetEnumerationControl", "V4L2_CTRL_TYPE_BUTTON", V4L2_CTRL_ID2CLASS (id));
    if (result < 0)
    {
        LOG_EX("Enumeration control of type V4L2_CTRL_TYPE_BUTTON cannot be set with V4L2_CTRL_CLASS_USER class");
    }
    emit SendSignalToUpdateWidgets();
}

void Camera::SetSliderEnumerationControlValue(int32_t id, int32_t val)
{
    SetExtControl(val, id, "SetEnumerationControl", "V4L2_CTRL_TYPE_INTEGER32", V4L2_CTRL_ID2CLASS (id));
}

void Camera::SetSliderEnumerationControlValue(int32_t id, int64_t val)
{
    SetExtControl(val, id, "SetEnumerationControl", "V4L2_CTRL_TYPE_INTEGER64", V4L2_CTRL_ID2CLASS (id));
}

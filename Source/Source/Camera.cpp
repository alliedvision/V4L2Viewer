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

    CloseDevice();
}

unsigned int Camera::GetReceivedFramesCount()
{
    return m_pFrameObserver->GetReceivedFramesCount();
}

unsigned int Camera::GetRenderedFramesCount()
{
    return m_pFrameObserver->GetRenderedFramesCount();
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
                LOG_EX("Camera::OpenDevice %s is a single-plane video capture device", deviceName.c_str());
            }
            else if (m_DeviceBufferType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
            {
                LOG_EX("Camera::OpenDevice %s is a multi-plane video capture device", deviceName.c_str());
            }
            else
            {
                LOG_EX("Camera::OpenDevice %s is no video capture device", deviceName.c_str());
            }


        }

        QueryControls(m_DeviceBufferType, deviceName);

        if (m_DeviceFileDescriptor == -1)
        {
            LOG_EX("Camera::OpenDevice open '%s' failed errno=%d=%s", deviceName.c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
        else
        {
            getAvtDeviceFirmwareVersion();

            LOG_EX("Camera::OpenDevice open %s OK", deviceName.c_str());

            m_DeviceName = deviceName;

            result = 0;
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

            QueryControls(subDeviceFileDescriptor, subDevice.toStdString());
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

    if (-1 != m_DeviceFileDescriptor)
    {
        if (-1 == close(m_DeviceFileDescriptor))
        {
            LOG_EX("Camera::CloseDevice close '%s' failed errno=%d=%s", m_DeviceName.c_str(), errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
        else
        {
            LOG_EX("Camera::CloseDevice close %s OK", m_DeviceName.c_str());
            result = 0;
        }
    }

    m_DeviceFileDescriptor = -1;

    for (auto const subDeviceFileDescriptor : m_SubDeviceFileDescriptors)
    {
        if (-1 == close(subDeviceFileDescriptor))
        {
            LOG_EX("Camera::CloseDevice sub-device close failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
        else
        {
            LOG_EX("Camera::CloseDevice sub-device close OK");
            result = 0;
        }
    }
    m_SubDeviceFileDescriptors.clear();
    m_SubDeviceBufferTypes.clear();

    return result;
}

void Camera::QueryControls(int fd, std::string const &name)
{
    const unsigned next_fl = V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_NEXT_COMPOUND;

    LOG_EX("Camera::QueryControls querying controls for %s", name.c_str());
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
            LOG_EX("Camera::DeviceDiscoveryStart open %s failed", deviceName.toLatin1().data());
        }
        else
        {
            v4l2_capability cap;

            LOG_EX("Camera::DeviceDiscoveryStart open %s found", deviceName.toLatin1().data());

            // query device capabilities
            if (-1 == iohelper::xioctl(fileDiscriptor, VIDIOC_QUERYCAP, &cap))
            {
                LOG_EX("Camera::DeviceDiscoveryStart %s is no V4L2 device", deviceName.toLatin1().data());
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
                LOG_EX("Camera::DeviceDiscoveryStart close %s failed", deviceName.toLatin1().data());
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
            LOG_EX("Camera::SubDeviceDiscoveryStart open %s failed", subDeviceName.toLatin1().data());
        }
        else
        {
            v4l2_capability cap;

            LOG_EX("Camera::SubDeviceDiscoveryStart open %s found", subDeviceName.toLatin1().data());

            // query sub-device capabilities
            if (-1 == iohelper::xioctl(fileDiscriptor, VIDIOC_QUERYCAP, &cap))
            {
                LOG_EX("Camera::SubDeviceDiscoveryStart %s is no V4L2 device", subDeviceName.toLatin1().data());
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
                LOG_EX("Camera::SubDeviceDiscoveryStart close %s failed", subDeviceName.toLatin1().data());
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

    LOG_EX("Camera::StartStreamChannel pixelFormat=%d, payloadSize=%d, width=%d, height=%d.", pixelFormat, payloadSize, width, height);

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
        LOG_EX("Camera::StartStreaming VIDIOC_STREAMON failed");
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
        LOG_EX("Camera::StopStreaming VIDIOC_STREAMOFF failed");
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
        LOG_EX("Camera::ReadPayloadSize VIDIOC_G_FMT OK =%d", fmt.fmt.pix.sizeimage);

        payloadSize = fmt.fmt.pix.sizeimage;

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadPayloadSize VIDIOC_G_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
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
        LOG_EX("Camera::ReadFrameSize VIDIOC_G_FMT OK =%dx%d", fmt.fmt.pix.width, fmt.fmt.pix.height);

        width = fmt.fmt.pix.width;
        height = fmt.fmt.pix.height;

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadFrameSize VIDIOC_G_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
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

        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        fmt.fmt.pix.bytesperline = 0;

        if (m_UseV4L2TryFmt)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_TRY_FMT, &fmt);
            if (result != 0)
            {
                LOG_EX("Camera::SetFrameSize VIDIOC_TRY_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
        }
        else
            result = 0;

        if (0 == result)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_S_FMT, &fmt);
            if (-1 != result)
            {
                LOG_EX("Camera::SetFrameSize VIDIOC_S_FMT OK =%dx%d", width, height);

                result = 0;
            }
        }
    }
    else
    {
        LOG_EX("Camera::SetFrameSize VIDIOC_G_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
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
        LOG_EX("Camera::SetWidth VIDIOC_G_FMT OK");

        fmt.fmt.pix.width = width;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        fmt.fmt.pix.bytesperline = 0;

        if (m_UseV4L2TryFmt)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_TRY_FMT, &fmt);
            if (result != 0)
            {
                LOG_EX("Camera::SetWidth VIDIOC_TRY_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
        }
        else
            result = 0;

        if (0 == result)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_S_FMT, &fmt);
            if (-1 != result)
            {
                LOG_EX("Camera::SetWidth VIDIOC_S_FMT OK =%d", width);

                result = 0;
            }
        }
    }
    else
    {
        LOG_EX("Camera::SetWidth VIDIOC_G_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
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
        LOG_EX("Camera::ReadWidth VIDIOC_G_FMT OK =%d", width);

        width = fmt.fmt.pix.width;

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadWidth VIDIOC_G_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
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
        LOG_EX("Camera::SetHeight VIDIOC_G_FMT OK");

        fmt.fmt.pix.height = height;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        fmt.fmt.pix.bytesperline = 0;

        if (m_UseV4L2TryFmt)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_TRY_FMT, &fmt);
            if (result != 0)
            {
                LOG_EX("Camera::SetHeight VIDIOC_TRY_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
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
                LOG_EX("Camera::SetHeight VIDIOC_S_FMT OK =%d", height);

                result = 0;
            }
        }
    }
    else
    {
        LOG_EX("Camera::SetHeight VIDIOC_G_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
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
        LOG_EX("Camera::ReadHeight VIDIOC_G_FMT OK =%d", fmt.fmt.pix.height);

        height = fmt.fmt.pix.height;

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadHeight VIDIOC_G_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::ReadFormats()
{
    int result = -1;
    v4l2_fmtdesc fmt;
    v4l2_frmsizeenum fmtsize;

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;
    while (iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_ENUM_FMT, &fmt) >= 0 && fmt.index <= 100)
    {
        std::string tmp = (char*)fmt.description;
        LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FMT index = %d", fmt.index);
        LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FMT type = %d", fmt.type);
        LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FMT pixel format = %d = %s", fmt.pixelformat, v4l2helper::ConvertPixelFormat2EnumString(fmt.pixelformat).c_str());
        LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FMT description = %s", fmt.description);

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

                LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum discrete width = %d", fmtsize.discrete.width);
                LOG_EX("Camera::ReadFormats size VIDIOC_ENUM_FRAMESIZES enum discrete height = %d", fmtsize.discrete.height);

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
                LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise min_width = %d", fmtsize.stepwise.min_width);
                LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise min_height = %d", fmtsize.stepwise.min_height);
                LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise max_width = %d", fmtsize.stepwise.max_width);
                LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise max_height = %d", fmtsize.stepwise.max_height);
                LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise step_width = %d", fmtsize.stepwise.step_width);
                LOG_EX("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise step_height = %d", fmtsize.stepwise.step_height);

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
        LOG_EX("Camera::SetPixelFormat VIDIOC_G_FMT OK");

        fmt.fmt.pix.pixelformat = pixelFormat;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        fmt.fmt.pix.bytesperline = 0;

        if (m_UseV4L2TryFmt)
        {
            result = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_TRY_FMT, &fmt);
            if (result != 0)
            {
                LOG_EX("Camera::SetPixelFormat VIDIOC_TRY_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
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
                LOG_EX("Camera::SetPixelFormat VIDIOC_S_FMT to %d OK", pixelFormat);
                result = 0;
            }
            else
            {
                LOG_EX("Camera::SetPixelFormat VIDIOC_S_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
        }

    }
    else
    {
        LOG_EX("Camera::SetPixelFormat VIDIOC_G_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::ReadPixelFormat(uint32_t &pixelFormat, uint32_t &bytesPerLine, QString &pfText)
{
    int result = -1;
    v4l2_format fmt;

    CLEAR(fmt);
    fmt.type = m_DeviceBufferType;

    if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_FMT, &fmt))
    {
        LOG_EX("Camera::ReadPixelFormat VIDIOC_G_FMT OK =%d", fmt.fmt.pix.pixelformat);

        pixelFormat = fmt.fmt.pix.pixelformat;
        bytesPerLine = fmt.fmt.pix.bytesperline;
        pfText = QString(v4l2helper::ConvertPixelFormat2EnumString(fmt.fmt.pix.pixelformat).c_str());

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadPixelFormat VIDIOC_G_FMT failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
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
        LOG_EX("Camera::EnumAllControlNewStyle fileDescriptor: %d", fileDescriptor);
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

                LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL id=%d=%s min=%d, max=%d, default=%d", qctrl.id, v4l2helper::ConvertControlID2String(qctrl.id).c_str(), qctrl.minimum, qctrl.maximum, qctrl.default_value);
                cidCount++;

                QString name = QString((const char*) qctrl.name);

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
                        LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s from file descriptor=%d", qctrl.name, fileDescriptor);
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
                        LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s from file descriptor=%d", qctrl.name, fileDescriptor);
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
                        LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s from file descriptor=%d", qctrl.name, fileDescriptor);
                        m_ControlIdToFileDescriptorMap[qctrl.id] = fileDescriptor;
                        emit SendBoolDataToEnumerationWidget(id, static_cast<bool>(value), name, unit, bIsReadOnly);
                    }
                }
                else if (qctrl.type == V4L2_CTRL_TYPE_BUTTON)
                {
                    int32_t id = qctrl.id;
                    LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s from file descriptor=%d", qctrl.name, fileDescriptor);
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
                        LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s from file descriptor=%d", qctrl.name, fileDescriptor);
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
                        LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL %s from file descriptor=%d", qctrl.name, fileDescriptor);
                        m_ControlIdToFileDescriptorMap[qctrl.id] = fileDescriptor;
                        emit SendListIntDataToEnumerationWidget(id, value, list, name, unit, bIsReadOnly);
                    }
                }
            }
            qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }

        if (0 == cidCount)
        {
            LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL returned error, no controls can be enumerated.");
        }
        else
        {
            LOG_EX("Camera::EnumAllControlNewStyle VIDIOC_QUERYCTRL: NumControls=%d", cidCount);
            result = 0;
        }
    }
    return result;
}

int Camera::ReadExtControl(int fileDescriptor, uint32_t &value, uint32_t controlID, const char *functionName, const char *controlName, uint32_t controlClass)
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

        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s OK, min=%d, max=%d, default=%d", functionName, controlName, ctrl.minimum, ctrl.maximum, ctrl.default_value);

        CLEAR(extCtrls);
        CLEAR(extCtrl);
        extCtrl.id = controlID;

        extCtrls.controls = &extCtrl;
        extCtrls.count = 1;
        extCtrls.ctrl_class = controlClass;

        if (-1 != iohelper::xioctl(fileDescriptor, VIDIOC_G_EXT_CTRLS, &extCtrls))
        {
            LOG_EX("Camera::%s VIDIOC_G_EXT_CTRLS %s OK =%d", functionName, controlName, extCtrl.value);

            value = extCtrl.value;

            result = 0;
        }
        else
        {
            LOG_EX("Camera::%s VIDIOC_G_CTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());

            result = -2;
        }
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());

        result = -2;
    }

    return result;
}

int Camera::SetExtControl(uint32_t value, uint32_t controlID, const char *functionName, const char *controlName, uint32_t controlClass)
{
    int result = -1;
    v4l2_ext_controls extCtrls;
    v4l2_ext_control extCtrl;

    CLEAR(extCtrls);
    CLEAR(extCtrl);
    extCtrl.id = controlID;
    extCtrl.value = value;

    extCtrls.controls = &extCtrl;
    extCtrls.count = 1;
    extCtrls.ctrl_class = controlClass;

    if (-1 != iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_TRY_EXT_CTRLS, &extCtrls))
    {
        if (-1 != iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_S_EXT_CTRLS, &extCtrls))
        {
            LOG_EX("Camera::%s VIDIOC_S_EXT_CTRLS %s to %d OK", functionName, controlName, value);
            result = 0;
        }
        else
        {
            LOG_EX("Camera::%s VIDIOC_S_EXT_CTRLS %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_TRY_EXT_CTRLS %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::SetExtControl(int32_t value, uint32_t controlID, const char *functionName, const char *controlName, uint32_t controlClass)
{
    int result = -1;
    v4l2_ext_controls extCtrls;
    v4l2_ext_control extCtrl;

    CLEAR(extCtrls);
    CLEAR(extCtrl);
    extCtrl.id = controlID;
    extCtrl.value = value;

    extCtrls.controls = &extCtrl;
    extCtrls.count = 1;
    extCtrls.ctrl_class = controlClass;

    if (-1 != iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_TRY_EXT_CTRLS, &extCtrls))
    {
        if (-1 != iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_S_EXT_CTRLS, &extCtrls))
        {
            LOG_EX("Camera::%s VIDIOC_S_EXT_CTRLS %s to %d OK", functionName, controlName, value);
            result = 0;
        }
        else
        {
            LOG_EX("Camera::%s VIDIOC_S_EXT_CTRLS %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_TRY_EXT_CTRLS %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::ReadExtControl(int fileDescriptor, int32_t &value, uint32_t controlID, const char *functionName, const char *controlName, uint32_t controlClass)
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

        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s OK, min=%d, max=%d, default=%d", functionName, controlName, ctrl.minimum, ctrl.maximum, ctrl.default_value);

        CLEAR(extCtrls);
        CLEAR(extCtrl);
        extCtrl.id = controlID;

        extCtrls.controls = &extCtrl;
        extCtrls.count = 1;
        extCtrls.ctrl_class = controlClass;

        if (-1 != iohelper::xioctl(fileDescriptor, VIDIOC_G_EXT_CTRLS, &extCtrls))
        {
            LOG_EX("Camera::%s VIDIOC_G_EXT_CTRLS %s OK =%d", functionName, controlName, extCtrl.value);

            value = extCtrl.value;

            result = 0;
        }
        else
        {
            LOG_EX("Camera::%s VIDIOC_G_CTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());

            result = -2;
        }
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());

        result = -2;
    }

    return result;
}

int Camera::SetExtControl(uint64_t value, uint32_t controlID, const char *functionName, const char *controlName, uint32_t controlClass)
{
    int result = -1;
    v4l2_ext_controls extCtrls;
    v4l2_ext_control extCtrl;

    CLEAR(extCtrls);
    CLEAR(extCtrl);
    extCtrl.id = controlID;
    extCtrl.value64 = value;

    extCtrls.controls = &extCtrl;
    extCtrls.count = 1;
    extCtrls.ctrl_class = controlClass;

    if (-1 != iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_TRY_EXT_CTRLS, &extCtrls))
    {
        if (-1 != iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_S_EXT_CTRLS, &extCtrls))
        {
            LOG_EX("Camera::%s VIDIOC_S_EXT_CTRLS %s to %d OK", functionName, controlName, value);
            result = 0;
        }
        else
        {
            LOG_EX("Camera::%s VIDIOC_S_EXT_CTRLS %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_TRY_EXT_CTRLS %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::ReadExtControl(int fileDescriptor, uint64_t &value, uint32_t controlID, const char *functionName, const char *controlName, uint32_t controlClass)
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

        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s OK, min=%d, max=%d, default=%d", functionName, controlName, ctrl.minimum, ctrl.maximum, ctrl.default_value);

        CLEAR(extCtrls);
        CLEAR(extCtrl);
        extCtrl.id = controlID;

        extCtrls.controls = &extCtrl;
        extCtrls.count = 1;
        extCtrls.ctrl_class = controlClass;

        if (-1 != iohelper::xioctl(fileDescriptor, VIDIOC_G_EXT_CTRLS, &extCtrls))
        {
            LOG_EX("Camera::%s VIDIOC_G_EXT_CTRLS %s OK =%d", functionName, controlName, extCtrl.value);

            value = extCtrl.value64;

            result = 0;
        }
        else
        {
            LOG_EX("Camera::%s VIDIOC_G_CTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());

            result = -2;
        }
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());

        result = -2;
    }

    return result;
}

int Camera::SetExtControl(int64_t value, uint32_t controlID, const char *functionName, const char *controlName, uint32_t controlClass)
{
    int result = -1;
    v4l2_ext_controls extCtrls;
    v4l2_ext_control extCtrl;

    CLEAR(extCtrls);
    CLEAR(extCtrl);
    extCtrl.id = controlID;
    extCtrl.value64 = value;

    extCtrls.controls = &extCtrl;
    extCtrls.count = 1;
    extCtrls.ctrl_class = controlClass;

    if (-1 != iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_TRY_EXT_CTRLS, &extCtrls))
    {
        if (-1 != iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_S_EXT_CTRLS, &extCtrls))
        {
            LOG_EX("Camera::%s VIDIOC_S_EXT_CTRLS %s to %d OK", functionName, controlName, value);
            result = 0;
        }
        else
        {
            LOG_EX("Camera::%s VIDIOC_S_EXT_CTRLS %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
        }
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_TRY_EXT_CTRLS %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
    }

    return result;
}

int Camera::ReadExtControl(int fileDescriptor, int64_t &value, uint32_t controlID, const char *functionName, const char *controlName, uint32_t controlClass)
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

        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s OK, min=%d, max=%d, default=%d", functionName, controlName, ctrl.minimum, ctrl.maximum, ctrl.default_value);

        CLEAR(extCtrls);
        CLEAR(extCtrl);
        extCtrl.id = controlID;

        extCtrls.controls = &extCtrl;
        extCtrls.count = 1;
        extCtrls.ctrl_class = controlClass;

        if (-1 != iohelper::xioctl(fileDescriptor, VIDIOC_G_EXT_CTRLS, &extCtrls))
        {
            LOG_EX("Camera::%s VIDIOC_G_EXT_CTRLS %s OK =%d", functionName, controlName, extCtrl.value);

            value = extCtrl.value64;

            result = 0;
        }
        else
        {
            LOG_EX("Camera::%s VIDIOC_G_CTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());

            result = -2;
        }
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());

        result = -2;
    }

    return result;
}

int Camera::ReadMinMax(uint32_t &min, uint32_t &max, uint32_t controlID, const char *functionName, const char *controlName)
{
    int result = -1;
    v4l2_queryctrl ctrl;

    CLEAR(ctrl);
    ctrl.id = controlID;

    if (iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_QUERYCTRL, &ctrl) >= 0)
    {
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s OK, min=%d, max=%d, default=%d", functionName, controlName, ctrl.minimum, ctrl.maximum, ctrl.default_value);
        min = ctrl.minimum;
        max = ctrl.maximum;
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
        result = -2;
    }

    return result;
}

int Camera::ReadMinMax(int32_t &min, int32_t &max, uint32_t controlID, const char *functionName, const char *controlName)
{
    int result = 0;
    v4l2_queryctrl ctrl;

    CLEAR(ctrl);
    ctrl.id = controlID;

    if (iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_QUERYCTRL, &ctrl) >= 0)
    {
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s OK, min=%d, max=%d, default=%d", functionName, controlName, ctrl.minimum, ctrl.maximum, ctrl.default_value);
        min = ctrl.minimum;
        max = ctrl.maximum;
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
        result = -2;
    }

    return result;
}

int Camera::ReadMinMax(int64_t &min, int64_t &max, uint32_t controlID, const char *functionName, const char *controlName)
{
    int result = 0;
    v4l2_query_ext_ctrl extCtrl;

    CLEAR(extCtrl);
    extCtrl.id = controlID;

    if (iohelper::xioctl(m_ControlIdToFileDescriptorMap[controlID], VIDIOC_QUERY_EXT_CTRL, &extCtrl) >= 0)
    {
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s OK, min=%d, max=%d, default=%d", functionName, controlName, extCtrl.minimum, extCtrl.maximum, extCtrl.default_value);

        min = extCtrl.minimum;
        max = extCtrl.maximum;
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
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
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s OK, step=%d", functionName, controlName, ctrl.step);
        step = ctrl.step;
    }
    else
    {
        LOG_EX("Camera::%s VIDIOC_QUERYCTRL %s failed errno=%d=%s", functionName, controlName, errno, v4l2helper::ConvertErrno2String(errno).c_str());
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
    flag = bool(value);
    return result;
}

int Camera::SetAutoExposure(bool autoexposure)
{
    if (autoexposure)
    {
        m_pAutoExposureReader->StartThread();
    }
    else
    {
        m_pAutoExposureReader->StopThread();
    }
    return SetExtControl(autoexposure, V4L2_CID_EXPOSURE_AUTO, "SetAutoExposure", "V4L2_CID_EXPOSURE_AUTO", V4L2_CTRL_CLASS_CAMERA);
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
    if (value)
    {
        m_pAutoGainReader->StartThread();
    }
    else
    {
        m_pAutoGainReader->StopThread();
    }
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

////////////////// Parameter ///////////////////

int Camera::ReadFrameRate(uint32_t &numerator, uint32_t &denominator, uint32_t width, uint32_t height, uint32_t pixelFormat)
{
    int result = -1;
    v4l2_streamparm parm;

    CLEAR(parm);
    parm.type = m_DeviceBufferType;

    if (iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_PARM, &parm) >= 0 && parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
    {
        v4l2_frmivalenum frmival;
        CLEAR(frmival);

        frmival.index = 0;
        frmival.pixel_format = pixelFormat;
        frmival.width = width;
        frmival.height = height;
        while (iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0)
        {
            frmival.index++;
            LOG_EX("Camera::ReadFrameRate type=%d", frmival.type);
        }
        if (frmival.index == 0)
        {
            LOG_EX("Camera::ReadFrameRate VIDIOC_ENUM_FRAMEINTERVALS failed");
        }

        v4l2_streamparm parm;
        CLEAR(parm);
        parm.type = m_DeviceBufferType;

        if (iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_PARM, &parm) >= 0)
        {
            numerator = parm.parm.capture.timeperframe.numerator;
            denominator = parm.parm.capture.timeperframe.denominator;
            LOG_EX("Camera::ReadFrameRate %d/%dOK", numerator, denominator);
        }
    }
    else
    {
        LOG_EX("Camera::ReadFrameRate VIDIOC_G_PARM V4L2_CAP_TIMEPERFRAME failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
        result = -2;
    }

    return result;
}

int Camera::SetFrameRate(uint32_t numerator, uint32_t denominator)
{
    int result = -1;
    v4l2_streamparm parm;

    CLEAR(parm);
    parm.type = m_DeviceBufferType;
    if (denominator != 0)
    {
        if (iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_PARM, &parm) >= 0)
        {
            parm.parm.capture.timeperframe.numerator = numerator;
            parm.parm.capture.timeperframe.denominator = denominator;
            if (-1 != iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_S_PARM, &parm))
            {
                LOG_EX("Camera::SetFrameRate VIDIOC_S_PARM to %d/%d (%.2f) OK", numerator, denominator, numerator / denominator);
                result = 0;
            }
            else
            {
                LOG_EX("Camera::SetFrameRate VIDIOC_S_PARM failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());
            }
        }
    }

    return result;
}

int Camera::ReadCrop(uint32_t &xOffset, uint32_t &yOffset, uint32_t &width, uint32_t &height)
{
    int result = -1;
    v4l2_crop crop;

    CLEAR(crop);
    crop.type = m_DeviceBufferType;

    if (iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_G_CROP, &crop) >= 0)
    {
        xOffset = crop.c.left;
        yOffset = crop.c.top;
        width = crop.c.width;
        height = crop.c.height;
        LOG_EX("Camera::ReadCrop VIDIOC_G_CROP x=%d, y=%d, w=%d, h=%d OK", xOffset, yOffset, width, height);

        result = 0;
    }
    else
    {
        LOG_EX("Camera::ReadCrop VIDIOC_G_CROP failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());

        result = -2;
    }

    return result;
}

int Camera::SetCrop(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height)
{
    int result = -1;
    v4l2_crop crop;

    CLEAR(crop);
    crop.type = m_DeviceBufferType;
    crop.c.left = xOffset;
    crop.c.top = yOffset;
    crop.c.width = width;
    crop.c.height = height;

    if (iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_S_CROP, &crop) >= 0)
    {
        LOG_EX("Camera::SetCrop VIDIOC_S_CROP %d, %d, %d, %d OK", xOffset, yOffset, width, height);

        result = 0;
    }
    else
    {
        LOG_EX("Camera::SetCrop VIDIOC_S_CROP failed errno=%d=%s", errno, v4l2helper::ConvertErrno2String(errno).c_str());

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
    int result = 0;
    v4l2_capability cap;

    // query device capabilities
    if (-1 == iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_QUERYCAP, &cap))
    {
        LOG_EX("Camera::GetCameraDriverName VIDIOC_QUERYCAP %s is no V4L2 device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        LOG_EX("Camera::GetCameraDriverName VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
    {
        LOG_EX("Camera::GetCameraDriverName %s is no video capture device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        LOG_EX("Camera::GetCameraDriverName VIDIOC_QUERYCAP %s driver name=%s\n", m_DeviceName.c_str(), (char*)cap.driver);
    }

    strText = (char*)cap.driver;

    return result;
}

int Camera::GetCameraDeviceName(std::string &strText)
{
    int result = 0;
    v4l2_capability cap;

    // query device capabilities
    if (-1 == iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_QUERYCAP, &cap))
    {
        LOG_EX("Camera::GetCameraDeviceName %s is no V4L2 device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        LOG_EX("Camera::GetCameraDeviceName VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
    {
        LOG_EX("Camera::GetCameraDeviceName %s is no video capture device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        LOG_EX("Camera::GetCameraDeviceName VIDIOC_QUERYCAP %s device name=%s\n", m_DeviceName.c_str(), (char*)cap.card);
    }

    strText = (char*)cap.card;

    return result;
}

int Camera::GetCameraBusInfo(std::string &strText)
{
    int result = 0;
    v4l2_capability cap;

    // query device capabilities
    if (-1 == iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_QUERYCAP, &cap))
    {
        LOG_EX("Camera::GetCameraBusInfo %s is no V4L2 device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        LOG_EX("Camera::GetCameraBusInfo VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
    {
        LOG_EX("Camera::GetCameraBusInfo %s is no video capture device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        LOG_EX("Camera::GetCameraBusInfo VIDIOC_QUERYCAP %s bus info=%s\n", m_DeviceName.c_str(), (char*)cap.bus_info);
    }

    strText = (char*)cap.bus_info;

    return result;
}

int Camera::GetCameraDriverVersion(std::string &strText)
{
    v4l2_capability cap;

    if (-1 == iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_QUERYCAP, &cap))
    {
        LOG_EX("Camera::GetCameraDriverVersion %s is no V4L2 device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        LOG_EX("Camera::GetCameraDriverVersion VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
    {
        LOG_EX("Camera::GetCameraDriverVersion %s is no video capture device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        LOG_EX("Camera::GetCameraDriverVersion VIDIOC_QUERYCAP %s device name=%s\n", m_DeviceName.c_str(), (char*)cap.card);
    }

    std::string cameraDriverInfo = (char*)cap.card;

    QString name = QString::fromStdString(cameraDriverInfo);
    QStringList list = name.split(" ");

    if (list.isEmpty())
    {
        LOG_EX("Couldn't get driver version for VIDIOC_QUERYCAP %s device name=%s\n", m_DeviceName.c_str(), (char*)cap.card);
        strText = "unknown";
        return -1;
    }

    if (list.back().contains('-'))
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
            LOG_EX("Couldn't get driver version for VIDIOC_QUERYCAP %s device name=%s\n", m_DeviceName.c_str(), (char*)cap.card);
            strText = "unknown";
            return -1;
        }

        QByteArray line = file.readLine();
        strText = line.toStdString();

        LOG_EX("Driver version for VIDIOC_QUERYCAP %s device name=%s was succesfully gathered = %s \n", m_DeviceName.c_str(), (char*)cap.card, strText.c_str());
    }
    else
    {
        LOG_EX("Couldn't get driver version for VIDIOC_QUERYCAP %s device name=%s\n", m_DeviceName.c_str(), (char*)cap.card);
        strText = "unknown";
        return -1;
    }

    return 0;
}

int Camera::GetCameraReadCapability(bool &flag)
{
    int result = 0;
    std::stringstream tmp;
    v4l2_capability cap;
    flag = false;

    // query device capabilities
    if (-1 == iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_QUERYCAP, &cap))
    {
        LOG_EX("Camera::GetCameraReadCapability %s is no V4L2 device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        LOG_EX("Camera::GetCameraReadCapability VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
    {
        LOG_EX("Camera::GetCameraReadCapability %s is no video capture device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        tmp << "0x" << std::hex << cap.capabilities << std::endl
            << "    Read/Write = " << ((cap.capabilities & V4L2_CAP_READWRITE)?"Yes":"No") << std::endl
            << "    Streaming = " << ((cap.capabilities & V4L2_CAP_STREAMING)?"Yes":"No");
        LOG_EX("Camera::GetCameraReadCapability VIDIOC_QUERYCAP %s driver version=%s\n", m_DeviceName.c_str(), tmp.str().c_str());

        if (cap.capabilities & V4L2_CAP_READWRITE)
            flag = true;
    }


    return result;
}

int Camera::GetCameraCapabilities(std::string &strText)
{
    int result = 0;
    std::stringstream tmp;
    v4l2_capability cap;

    // query device capabilities
    if (-1 == iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_QUERYCAP, &cap))
    {
        LOG_EX("Camera::GetCameraCapabilities %s is no V4L2 device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        LOG_EX("Camera::GetCameraCapabilities VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
    }

    std::string driverName((char*)cap.driver);
    std::string avtDriverName = "avt";
    if(driverName.find(avtDriverName) != std::string::npos)
    {
        m_IsAvtCamera = true;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
    {
        LOG_EX("Camera::GetCameraCapabilities %s is no video capture device\n", m_DeviceName.c_str());
        return -1;
    }
    else
    {
        tmp << "0x" << std::hex << cap.capabilities << std::endl
            << "    Read/Write = " << ((cap.capabilities & V4L2_CAP_READWRITE)?"Yes":"No") << std::endl
            << "    Streaming = " << ((cap.capabilities & V4L2_CAP_STREAMING)?"Yes":"No");
        LOG_EX("Camera::GetCameraCapabilities VIDIOC_QUERYCAP %s driver version=%s\n", m_DeviceName.c_str(), tmp.str().c_str());
    }

    strText = tmp.str();

    return result;
}

int Camera::GetSubDevicesCapabilities(std::string &strText)
{
    int result = 0;

    for (auto const fileDescriptor : m_SubDeviceFileDescriptors)
    {
        std::stringstream tmp;
        v4l2_capability cap;

        // query device capabilities
        if (-1 == iohelper::xioctl(fileDescriptor, VIDIOC_QUERYCAP, &cap))
        {
            LOG_EX("Camera::GetCameraCapabilities %s is no V4L2 device\n", m_DeviceName.c_str());
            return -1;
        }
        else
        {
            LOG_EX("Camera::GetCameraCapabilities VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
        }

        std::string driverName((char*) cap.driver);
        std::string avtDriverName = "avt";
        if (driverName.find(avtDriverName) != std::string::npos)
        {
            m_IsAvtCamera = true;
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
        {
            LOG_EX("Camera::GetCameraCapabilities %s is no video capture device\n", m_DeviceName.c_str());
            return -1;
        }
        else
        {
            tmp << "0x" << std::hex << cap.capabilities << std::endl << "    Read/Write = " << ((cap.capabilities & V4L2_CAP_READWRITE) ? "Yes" : "No") << std::endl << "    Streaming = " << ((cap.capabilities & V4L2_CAP_STREAMING) ? "Yes" : "No");
            LOG_EX("Camera::GetCameraCapabilities VIDIOC_QUERYCAP %s driver version=%s\n", m_DeviceName.c_str(), tmp.str().c_str());
        }

        strText += tmp.str();
    }

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
                    ss << (unsigned)specialVersion << "." << (unsigned)majorVersion << "." << (unsigned)minorVersion << "." << (unsigned)patchVersion;
                    result = ss.str();
                }
            }
        }
    }

    return result;
}


std::string Camera::getAvtDeviceTemperature()
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
            const int BCRM_DEV_TEMPERATURE = 0x0310;

            uint16_t nBCRMAddress = 0;
            int res = ReadRegister(CCI_BCRM_REG, &nBCRMAddress, sizeof(nBCRMAddress), true);

            if (res >= 0)
            {
                int32_t deviceTemperture = 0;
                res = ReadRegister((__u32)nBCRMAddress + BCRM_DEV_TEMPERATURE, &deviceTemperture, sizeof(deviceTemperture), true);

                if (res >= 0)
                {
                    char buff[32];
                    snprintf(buff, sizeof(buff), "%.1f", (double)deviceTemperture/10);
                    result = std::string(buff);
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

    if(m_pFrameObserver)
    {
        // dummy call to set m_isAvtCamera
        std::string dummy;
        GetCameraCapabilities(dummy);

        if(m_IsAvtCamera)
        {
            v4l2_stats_t stream_stats;
            CLEAR(stream_stats);
            if (ioctl(m_DeviceFileDescriptor, VIDIOC_STREAMSTAT, &stream_stats) >= 0)
            {
                FramesCount = stream_stats.frames_count;
                PacketCRCError = stream_stats.packet_crc_error;
                FramesUnderrun = stream_stats.frames_underrun;
                FramesIncomplete = stream_stats.frames_incomplete;
                CurrentFrameRate = (stream_stats.current_frame_interval > 0) ? (double)stream_stats.current_frame_count / ((double)stream_stats.current_frame_interval / 1000000.0)
                                                                            : 0;
                result = true;
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

    v4l2_i2c i2c_reg;
    i2c_reg.register_address = (__u32)nRegAddr;
    i2c_reg.register_size = (__u32)2;
    i2c_reg.num_bytes = (__u32)nBufferSize;
    i2c_reg.ptr_buffer = (char*)pBuffer;

    int res = iohelper::xioctl(m_DeviceFileDescriptor, VIDIOC_R_I2C, &i2c_reg);

    if (res >= 0)
    {
        iRet = 0;
    }

    // Values in BCM register are Big Endian -> swap bytes
    if(bConvertEndianess && (QSysInfo::ByteOrder == QSysInfo::LittleEndian))
    {
        reverseBytes(pBuffer, nBufferSize);
    }

    return iRet;
}

int Camera::WriteRegister(uint16_t nRegAddr, void* pBuffer, uint32_t nBufferSize, bool bConvertEndianess)
{
    int iRet = -1;

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
    }

    return iRet;
}

void Camera::PassGainValue()
{
    int32_t value = 0;
    ReadExtControl(value, V4L2_CID_GAIN, "ReadGain", "V4L2_CID_GAIN", V4L2_CTRL_CLASS_USER);
    emit PassAutoGainValue(value);
}

void Camera::PassExposureValue()
{
    int32_t value = 0;
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
        LOG_EX("Camera::SetEnumerationControlValueIntList: %d", fileDescriptor);

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
                        LOG_EX("Camera::SetEnumerationControlValueIntList: using file descriptor %d to set %s", fileDescriptor, queryMenu.name);
                        result = SetExtControl(queryMenu.index, id, "SetEnumerationControl", "V4L2_CTRL_TYPE_MENU", V4L2_CTRL_ID2CLASS(id));
                        if (result < 0)
                        {
                            LOG_EX("Enumeration control %s cannot be set with V4L2_CTRL_CLASS_USER class", (const char*) qctrl.name);
                        }
                        emit SendSignalToUpdateWidgets();
                        break;
                    }
                }
            }
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
        LOG_EX("Camera::SetEnumerationControlValueList: %d", fileDescriptor);

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
                        LOG_EX("Camera::SetEnumerationControlValueList: using file descriptor %d to set %s", fileDescriptor, queryMenu.name);
                        result = SetExtControl(queryMenu.index, id, "SetEnumerationControl", "V4L2_CTRL_TYPE_MENU", V4L2_CTRL_ID2CLASS(id));
                        if (result < 0)
                        {
                            LOG_EX("Enumeration control %s cannot be set with V4L2_CTRL_CLASS_USER class", str);
                        }
                        emit SendSignalToUpdateWidgets();
                        break;
                    }
                }
            }
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

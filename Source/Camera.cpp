/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        CameraObserver.cpp

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

#include "V4l2Helper.h"
#include "Camera.h"
#include "Logger.h"
#include <stdio.h>
#include <sstream>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "videodev2_av.h"

#include "Helper.h"
#include "FrameObserverMMAP.h"
#include "FrameObserverUSER.h"

#include <QStringList>

namespace AVT {
namespace Tools {
namespace Examples {

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////    

Camera::Camera()
    : m_nFileDescriptor(-1)
    , m_BlockingMode(false)
    , m_ShowFrames(true)
{
    connect(&m_DeviceDiscoveryCallbacks, SIGNAL(OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &, const QString &)), this, SLOT(OnCameraListChanged(const int &, unsigned int, unsigned long long, const QString &, const QString &)));
}

Camera::~Camera()
{
    m_DeviceDiscoveryCallbacks.SetTerminateFlag();
    if (NULL != m_StreamCallbacks.data())
	m_StreamCallbacks->StopStream();

    CloseDevice();
}

unsigned int Camera::GetReceivedFramesCount()
{
    return m_StreamCallbacks->GetReceivedFramesCount();
}

unsigned int Camera::GetDroppedFramesCount()
{
    return m_StreamCallbacks->GetDroppedFramesCount();
}

int Camera::OpenDevice(std::string &deviceName, bool blockingMode, bool mmapBuffer)
{
	int result = -1;
	
        m_BlockingMode = blockingMode;

	if (mmapBuffer)
	{
	    m_StreamCallbacks = QSharedPointer<FrameObserverMMAP>(new FrameObserverMMAP(m_ShowFrames));
	}
	else
	{
	    m_StreamCallbacks = QSharedPointer<FrameObserverUSER>(new FrameObserverUSER(m_ShowFrames));
	}
	connect(m_StreamCallbacks.data(), SIGNAL(OnFrameReady_Signal(const QImage &, const unsigned long long &)), this, SLOT(OnFrameReady(const QImage &, const unsigned long long &)));
	connect(m_StreamCallbacks.data(), SIGNAL(OnFrameID_Signal(const unsigned long long &)), this, SLOT(OnFrameID(const unsigned long long &)));
	connect(m_StreamCallbacks.data(), SIGNAL(OnRecordFrame_Signal(const unsigned long long &, const unsigned long long &)), this, SLOT(OnRecordFrame(const unsigned long long &, const unsigned long long &)));
    	connect(m_StreamCallbacks.data(), SIGNAL(OnDisplayFrame_Signal(const unsigned long long &)), this, SLOT(OnDisplayFrame(const unsigned long long &)));
    	connect(m_StreamCallbacks.data(), SIGNAL(OnMessage_Signal(const QString &)), this, SLOT(OnMessage(const QString &)));
    	connect(m_StreamCallbacks.data(), SIGNAL(OnError_Signal(const QString &)), this, SLOT(OnError(const QString &)));


	if (-1 == m_nFileDescriptor)
	{
                if (m_BlockingMode)
                   m_nFileDescriptor = open(deviceName.c_str(), O_RDWR, 0);
                else
                   m_nFileDescriptor = open(deviceName.c_str(), O_RDWR | O_NONBLOCK, 0);

		if (m_nFileDescriptor == -1) 
		{
			Logger::LogEx("Camera::OpenDevice open '%s' failed errno=%d=%s", deviceName.c_str(), errno, V4l2Helper::ConvertErrno2String(errno).c_str());
			emit OnCameraError_Signal("OpenDevice: open '" + QString(deviceName.c_str()) + "' failed errno=" + QString((int)errno) + "=" + QString(V4l2Helper::ConvertErrno2String(errno).c_str()) + ".");
		}
		else
		{
			Logger::LogEx("Camera::OpenDevice open %s OK", deviceName.c_str());
			emit OnCameraMessage_Signal("OpenDevice: open " + QString(deviceName.c_str()) + " OK");
	
			m_DeviceName = deviceName;
	
			result = 0;
		}
	}
	else
	{
		Logger::LogEx("Camera::OpenDevice open %s failed because %s is already open", deviceName.c_str(), m_DeviceName.c_str());
		emit OnCameraError_Signal("OpenDevice: open " + QString(deviceName.c_str()) + " failed because " + QString(m_DeviceName.c_str()) + " is already open");
	}
	
	return result;
}

int Camera::CloseDevice()
{
    int result = -1;

    if (-1 != m_nFileDescriptor)
    {
        if (-1 == close(m_nFileDescriptor))
		{
			Logger::LogEx("Camera::CloseDevice close '%s' failed errno=%d=%s", m_DeviceName.c_str(), errno, V4l2Helper::ConvertErrno2String(errno).c_str());
			emit OnCameraError_Signal("CloseDevice: close '" + QString(m_DeviceName.c_str()) + "' failed errno=" + QString((int)errno) + "+" + QString(V4l2Helper::ConvertErrno2String(errno).c_str()) + ".");
		}
		else
		{
			Logger::LogEx("Camera::CloseDevice close %s OK", m_DeviceName.c_str());
			emit OnCameraMessage_Signal("CloseDevice: close " + QString(m_DeviceName.c_str()) + " OK");
		
			result = 0;
		}
    }
    
    m_nFileDescriptor = -1;
	
    return result;
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

// Event will be called when the a frame is recorded
void Camera::OnRecordFrame(const unsigned long long &frameID, const unsigned long long &framesInQueue)
{
    emit OnCameraRecordFrame_Signal(frameID, framesInQueue);
}

// Event will be called when the a frame is displayed
void Camera::OnDisplayFrame(const unsigned long long &frameID)
{
    emit OnCameraDisplayFrame_Signal(frameID);
}

// Event will be called when for text notification
void Camera::OnMessage(const QString &msg)
{
    emit OnCameraMessage_Signal(msg);
}

// Event will be called when for text notification
void Camera::OnError(const QString &msg)
{
    emit OnCameraError_Signal(msg);
}

// The event handler to set or remove devices 
void Camera::OnCameraListChanged(const int &reason, unsigned int cardNumber, unsigned long long deviceID, const QString &deviceName, const QString &info)
{
    emit OnCameraListChanged_Signal(reason, cardNumber, deviceID, deviceName, info);
}

/********************************************************************************/
// Device discovery
/********************************************************************************/

int Camera::DeviceDiscoveryStart()
{
    uint32_t device_count = 0;
    int nResult = 0;
	QString deviceName;
	int fileDiscriptor = -1;
	
	do
	{
		deviceName.sprintf("/dev/video%d", device_count);
		
		if ((fileDiscriptor = open(deviceName.toStdString().c_str(), O_RDWR)) == -1) 
		{
			Logger::LogEx("Camera::DeviceDiscovery open %s failed", deviceName.toAscii().data());
			//emit OnCameraError_Signal("DeviceDiscovery: open " + deviceName + " failed");
		}
		else
		{
			struct v4l2_capability cap;
						
			Logger::LogEx("Camera::DeviceDiscovery open %s found", deviceName.toAscii().data());
			emit OnCameraMessage_Signal("DeviceDiscovery: open " + deviceName + " found");
			
			// queuery device capabilities
			if (-1 == V4l2Helper::xioctl(fileDiscriptor, VIDIOC_QUERYCAP, &cap)) 
			{
				Logger::LogEx("Camera::DeviceDiscovery %s is no V4L2 device", deviceName.toAscii().data());
				emit OnCameraError_Signal("DeviceDiscovery: " + deviceName + " is no V4L2 device");
			}
			else
			{
				if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
				{
					Logger::LogEx("Camera::DeviceDiscovery %s is no video capture device", deviceName.toAscii().data());
					emit OnCameraError_Signal("DeviceDiscovery: " + deviceName + " is no video capture device");
				}
				else
				{
				    emit OnCameraListChanged_Signal(UpdateTriggerPluggedIn, 0, device_count, deviceName, (const char*)cap.card);
				}
			}
			
			
			if (-1 == close(fileDiscriptor))
			{
				Logger::LogEx("Camera::DeviceDiscoveryStart close %s failed", deviceName.toAscii().data());
				emit OnCameraError_Signal("DeviceDiscoveryStart: close " + deviceName + " failed");
			}
			else
			{
				Logger::LogEx("Camera::DeviceDiscovery close %s OK", deviceName.toAscii().data());
				//emit OnCameraMessage_Signal("DeviceDiscovery: close " + deviceName + " OK");
			}
		}
		
		device_count++;
	}
	while(device_count < 20);
	
    //m_DeviceDiscoveryCallbacks.Start();

    return 0;
}

int Camera::DeviceDiscoveryStop()
{
    Logger::LogEx("Device Discovery stopped.");

    //m_DeviceDiscoveryCallbacks.Stop();

    return 0;
}
    
/********************************************************************************/
// SI helper
/********************************************************************************/


int Camera::StartStreamChannel(const char* csvFilename,
                               uint32_t pixelformat, uint32_t payloadsize, uint32_t width, uint32_t height, 
			       uint32_t bytesPerLine, void *pPrivateData,
			       uint32_t enableLogging, uint32_t logFrameStart, uint32_t logFrameEnd,
			       uint32_t dumpFrameStart, uint32_t dumpFrameEnd)
{
    int nResult = 0;
    
    if (NULL != csvFilename && csvFilename[0] != 0)
	ReadCSVFile(csvFilename, m_rCSVData);

    
    Logger::LogEx("Camera::StartStreamChannel pixelformat=%d, payloadsize=%d, width=%d, height=%d.", pixelformat, payloadsize, width, height);
	
    m_StreamCallbacks->StartStream(m_BlockingMode, m_nFileDescriptor, pixelformat, 
				   payloadsize, width, height, bytesPerLine,
				   enableLogging, logFrameStart, logFrameEnd,
				   dumpFrameStart, dumpFrameEnd, m_rCSVData);

    m_StreamCallbacks->ResetDroppedFramesCount();

    // start stream returns always success
    Logger::LogEx("Camera::StartStreamChannel OK.");
    emit OnCameraMessage_Signal("Camera::StartStreamChannel OK.");
	
    return nResult;
}

int Camera::StopStreamChannel()
{
    int nResult = 0;
    
    Logger::LogEx("Camera::StopStreamChannel started.");
    
    nResult = m_StreamCallbacks->StopStream();

    if (nResult == 0)
    {
       Logger::LogEx("Camera::StopStreamChannel OK.");
       emit OnCameraMessage_Signal("Camera::StopStreamChannel OK.");
    } 
    else
    {
       Logger::LogEx("Camera::StopStreamChannel failed.");
       emit OnCameraError_Signal("Camera::StopStreamChannel failed.");
    }

    m_rCSVData.resize(0);
    
    return nResult;
}

/*********************************************************************************************************/
// DCI commands
/*********************************************************************************************************/

int Camera::StartStreaming()
{
    int result = -1;
    v4l2_buf_type type;
	
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_STREAMON, &type))
    {
        Logger::LogEx("Camera::StartStreaming VIDIOC_STREAMON failed");
        emit OnCameraError_Signal("StartStreaming: VIDIOC_STREAMON failed.");
    }
    else
    {
        Logger::LogEx("Camera::StartStreaming VIDIOC_STREAMON OK");
        emit OnCameraMessage_Signal("StartStreaming: VIDIOC_STREAMON OK.");
		
	result = 0;
    }
	
    return result;
}

int Camera::StopStreaming()
{
    int result = -1;
    v4l2_buf_type type;
	
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
    if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_STREAMOFF, &type))
    {
        Logger::LogEx("Camera::StopStreaming VIDIOC_STREAMOFF failed");
        emit OnCameraError_Signal("StopStreaming: VIDIOC_STREAMOFF failed.");
    }
    else
    {
        Logger::LogEx("Camera::StopStreaming VIDIOC_STREAMOFF OK");
        emit OnCameraMessage_Signal("StopStreaming: VIDIOC_STREAMOFF OK.");
		
	result = 0;
    }
	
    return result;
}

int Camera::ReadPayloadsize(uint32_t &payloadsize)
{
    int result = -1;
    v4l2_format fmt;
	
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_FMT, &fmt))
    {                
        Logger::LogEx("Camera::ReadPayloadsize VIDIOC_G_FMT OK =%d", fmt.fmt.pix.sizeimage);
        emit OnCameraMessage_Signal(QString("ReadPayloadsize VIDIOC_G_FMT: OK =%1.").arg(fmt.fmt.pix.sizeimage));
		
	payloadsize = fmt.fmt.pix.sizeimage;
		
	result = 0;
    }
    else
    {
	Logger::LogEx("Camera::ReadPayloadsize VIDIOC_G_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("ReadPayloadsize VIDIOC_G_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
    }
	
    return result;
}

int Camera::ReadFrameSize(uint32_t &width, uint32_t &height)
{
    int result = -1;
    v4l2_format fmt;
	
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_FMT, &fmt))
    {       
        Logger::LogEx("Camera::ReadFrameSize VIDIOC_G_FMT OK =%dx%d", fmt.fmt.pix.width, fmt.fmt.pix.height);
        emit OnCameraMessage_Signal(QString("ReadFrameSize VIDIOC_G_FMT: OK =%1x%2.").arg(fmt.fmt.pix.width).arg(fmt.fmt.pix.height));
         
	width = fmt.fmt.pix.width;
	height = fmt.fmt.pix.height;
		
	result = 0;
    }
    else
    {
	Logger::LogEx("Camera::ReadFrameSize VIDIOC_G_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("ReadFrameSize VIDIOC_G_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
    }
	
    return result;
}

int Camera::SetFrameSize(uint32_t width, uint32_t height)
{
    int result = -1;
    v4l2_format fmt;
	
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_FMT, &fmt))
    {                
	Logger::LogEx("Camera::SetFrameSize VIDIOC_G_FMT OK");
	emit OnCameraMessage_Signal(QString("SetFrameSize VIDIOC_G_FMT: OK."));

	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;
	fmt.fmt.pix.bytesperline = 0;

	if (0 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_TRY_FMT, &fmt))
	{       
	    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_FMT, &fmt))
	    {                
		Logger::LogEx("Camera::SetFrameSize VIDIOC_S_FMT OK =%dx%d", width, height);
		emit OnCameraMessage_Signal(QString("SetFrameSize VIDIOC_S_FMT: OK =%1x%2.").arg(fmt.fmt.pix.width).arg(fmt.fmt.pix.height));

		result = 0;
	    }
	}
	else
	{
	    Logger::LogEx("Camera::SetFrameSize VIDIOC_TRY_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
	    emit OnCameraError_Signal(QString("SetFrameSize VIDIOC_TRY_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
    }
    else
    {
	Logger::LogEx("Camera::SetFrameSize VIDIOC_G_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
	emit OnCameraError_Signal(QString("SetFrameSize VIDIOC_G_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
    }
	
    return result;
}

int Camera::SetWidth(uint32_t width)
{
    int result = -1;
    v4l2_format fmt;
	
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_FMT, &fmt))
    {       
	Logger::LogEx("Camera::SetWidth VIDIOC_G_FMT OK");
	emit OnCameraMessage_Signal(QString("SetWidth VIDIOC_G_FMT: OK."));
		
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;
	fmt.fmt.pix.bytesperline = 0;

	if (0 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_TRY_FMT, &fmt))
	{       
	    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_FMT, &fmt))
	    {             
		Logger::LogEx("Camera::SetWidth VIDIOC_S_FMT OK =%d", width);
		emit OnCameraMessage_Signal(QString("SetWidth VIDIOC_S_FMT: OK =%1.").arg(width));   
			
		result = 0;
	    }
	}
	else
        {
            Logger::LogEx("Camera::SetWidth VIDIOC_TRY_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
            emit OnCameraError_Signal(QString("SetWidth VIDIOC_TRY_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
    }
    else
    {
	Logger::LogEx("Camera::SetWidth VIDIOC_G_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetWidth VIDIOC_G_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
    }
	
    return result;
}

int Camera::ReadWidth(uint32_t &width)
{
    int result = -1;
    v4l2_format fmt;
	
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_FMT, &fmt))
    {                
        Logger::LogEx("Camera::ReadWidth VIDIOC_G_FMT OK =%d", width);
        emit OnCameraMessage_Signal(QString("ReadWidth VIDIOC_G_FMT: OK =%1.").arg(width));

	width = fmt.fmt.pix.width;
		
	result = 0;
    }
    else
    {
	Logger::LogEx("Camera::ReadWidth VIDIOC_G_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("ReadWidth VIDIOC_G_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
    }
	
    return result;
}

int Camera::SetHeight(uint32_t height)
{
    int result = -1;
    v4l2_format fmt;
	
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_FMT, &fmt))
    {                
	Logger::LogEx("Camera::SetHeight VIDIOC_G_FMT OK");
	emit OnCameraMessage_Signal(QString("SetHeight VIDIOC_G_FMT: OK."));

	fmt.fmt.pix.height = height;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;
	fmt.fmt.pix.bytesperline = 0;

	if (0 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_TRY_FMT, &fmt))
	{       
	    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_FMT, &fmt))
	    {
            	Logger::LogEx("Camera::SetHeight VIDIOC_S_FMT OK =%d", height);
                emit OnCameraMessage_Signal(QString("SetHeight VIDIOC_S_FMT: OK =%1.").arg(height));
                
                result = 0;
	    }
	}
	else
        {
            Logger::LogEx("Camera::SetHeight VIDIOC_TRY_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
            emit OnCameraError_Signal(QString("SetHeight VIDIOC_TRY_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
        }
    }
    else
    {
	Logger::LogEx("Camera::SetHeight VIDIOC_G_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetHeight VIDIOC_G_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
    }
	
    return result;
}

int Camera::ReadHeight(uint32_t &height)
{
    int result = -1;
    v4l2_format fmt;
	
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_FMT, &fmt))
    {       
        Logger::LogEx("Camera::ReadHeight VIDIOC_G_FMT OK =%d", fmt.fmt.pix.height);
        emit OnCameraMessage_Signal(QString("ReadHeight VIDIOC_G_FMT: OK =%1.").arg(fmt.fmt.pix.height));
         
	height = fmt.fmt.pix.height;
		
	result = 0;
    }
    else
    {
	Logger::LogEx("Camera::ReadHeight VIDIOC_G_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("ReadHeight VIDIOC_G_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
    }
	
    return result;
}

int Camera::ReadFormats()
{
    int result = -1;
    v4l2_fmtdesc fmt;
    v4l2_frmsizeenum fmtsize;
	
    int x=V4L2_PIX_FMT_RGB565;
    int z=V4L2_PIX_FMT_UYVY;
	
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_ENUM_FMT, &fmt) >= 0 && fmt.index <= 100)
    {
	std::string tmp = (char*)fmt.description;
	Logger::LogEx("Camera::ReadFormats VIDIOC_ENUM_FMT index = %d", fmt.index);
	emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FMT: index = " + QString("%1").arg(fmt.index) + ".");
	Logger::LogEx("Camera::ReadFormats VIDIOC_ENUM_FMT type = %d", fmt.type);
	emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FMT: type = " + QString("%1").arg(fmt.type) + ".");
	Logger::LogEx("Camera::ReadFormats VIDIOC_ENUM_FMT pixelformat = %d = %s", fmt.pixelformat, V4l2Helper::ConvertPixelformat2EnumString(fmt.pixelformat).c_str());
	emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FMT: pixelformat = " + QString("%1").arg(fmt.pixelformat) + " = " + QString(V4l2Helper::ConvertPixelformat2EnumString(fmt.pixelformat).c_str()) + ".");
	Logger::LogEx("Camera::ReadFormats VIDIOC_ENUM_FMT description = %s", fmt.description);
	emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FMT: description = " + QString(tmp.c_str()) + ".");
		
	emit OnCameraPixelformat_Signal(QString("%1").arg(QString(V4l2Helper::ConvertPixelformat2String(fmt.pixelformat).c_str())));
				
	CLEAR(fmtsize);
	fmtsize.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmtsize.pixel_format = fmt.pixelformat;
	fmtsize.index = 0;
	while (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_ENUM_FRAMESIZES, &fmtsize) >= 0 && fmtsize.index <= 100)
	{              
	    if (fmtsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
	    {
		v4l2_frmivalenum fmtival;
				
		Logger::LogEx("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum discrete width = %d", fmtsize.discrete.width);
		emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FRAMESIZES size enum discrete width = " + QString("%1").arg(fmtsize.discrete.width) + ".");
		Logger::LogEx("Camera::ReadFormats size VIDIOC_ENUM_FRAMESIZES enum discrete height = %d", fmtsize.discrete.height);
		emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FRAMESIZES size enum discrete height = " + QString("%1").arg(fmtsize.discrete.height) + ".");
		
		emit OnCameraFramesize_Signal(QString("disc:%1x%2").arg(fmtsize.discrete.width).arg(fmtsize.discrete.height));
				
		CLEAR(fmtival);
		fmtival.index = 0;
		fmtival.pixel_format = fmt.pixelformat;
		fmtival.width = fmtsize.discrete.width;
		fmtival.height = fmtsize.discrete.height;
		while (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_ENUM_FRAMEINTERVALS, &fmtival) >= 0)
		{
		    // TODO
		    //Logger::LogEx("Camera::ReadFormat ival type= %d", fmtival.type);
		    //emit OnCameraMessage_Signal("ReadFormatsiz ival type = " + QString("%1").arg(fmtival.type) + ".");
		    fmtival.index++;
		}
	    } 
	    else if (fmtsize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
	    {
		Logger::LogEx("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise min_width = %d", fmtsize.stepwise.min_width);
		emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise min_width = " + QString("%1").arg(fmtsize.stepwise.min_width) + ".");
		Logger::LogEx("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise min_height = %d", fmtsize.stepwise.min_height);
		emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise min_height = " + QString("%1").arg(fmtsize.stepwise.min_height) + ".");
		Logger::LogEx("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise max_width = %d", fmtsize.stepwise.max_width);
		emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise max_width = " + QString("%1").arg(fmtsize.stepwise.max_width) + ".");
		Logger::LogEx("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise max_height = %d", fmtsize.stepwise.max_height);
		emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise max_height = " + QString("%1").arg(fmtsize.stepwise.max_height) + ".");
		Logger::LogEx("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise step_width = %d", fmtsize.stepwise.step_width);
		emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise step_width = " + QString("%1").arg(fmtsize.stepwise.step_width) + ".");
		Logger::LogEx("Camera::ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise step_height = %d", fmtsize.stepwise.step_height);
		emit OnCameraMessage_Signal("ReadFormats VIDIOC_ENUM_FRAMESIZES size enum stepwise step_height = " + QString("%1").arg(fmtsize.stepwise.step_height) + ".");
		
		emit OnCameraFramesize_Signal(QString("min:%1x%2,max:%3x%4,step:%5x%6").arg(fmtsize.stepwise.min_width).arg(fmtsize.stepwise.min_height).arg(fmtsize.stepwise.max_width).arg(fmtsize.stepwise.max_height).arg(fmtsize.stepwise.step_width).arg(fmtsize.stepwise.step_height));
	    }
			
	    result = 0;
			
	    fmtsize.index++;
	}
		
	if (fmtsize.index >= 100)
	{
	    Logger::LogEx("Camera::ReadFormats no VIDIOC_ENUM_FRAMESIZES never terminated with EINVAL within 100 loops.");
	    emit OnCameraError_Signal("ReadFormats: no VIDIOC_ENUM_FRAMESIZES never terminated with EINVAL within 100 loops.");
        }
		
	fmt.index++;
    }

    if (fmt.index >= 100)
    {
	Logger::LogEx("Camera::ReadFormats no VIDIOC_ENUM_FMT never terminated with EINVAL within 100 loops.");
	emit OnCameraError_Signal("ReadFormats: get VIDIOC_ENUM_FMT never terminated with EINVAL within 100 loops.");
    }
	
    return result;
}

int Camera::SetPixelformat(uint32_t pixelformat, QString pfText)
{
    int result = -1;
    v4l2_format fmt;
	
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_FMT, &fmt))
    {                
	Logger::LogEx("Camera::SetPixelformat VIDIOC_G_FMT OK");
	emit OnCameraMessage_Signal(QString("SetPixelformat VIDIOC_G_FMT: OK."));

	fmt.fmt.pix.pixelformat = pixelformat;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
	fmt.fmt.pix.bytesperline = 0;

	if (0 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_TRY_FMT, &fmt))
	{
	    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_FMT, &fmt))
	    {                
                Logger::LogEx("Camera::SetPixelformat VIDIOC_S_FMT to %d OK", pixelformat);
                emit OnCameraMessage_Signal(QString("SetPixelformat VIDIOC_S_FMT: to %1 OK.").arg(pixelformat));

                result = 0;
	    }
            else
            {
                Logger::LogEx("Camera::SetPixelformat VIDIOC_S_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
                emit OnCameraError_Signal(QString("SetPixelformat VIDIOC_S_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
            }
        }
	else
        {
            Logger::LogEx("Camera::SetPixelformat VIDIOC_TRY_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
            emit OnCameraError_Signal(QString("SetPixelformat VIDIOC_TRY_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
        }
    }
    else
    {
	Logger::LogEx("Camera::SetPixelformat VIDIOC_G_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetPixelformat VIDIOC_G_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
    }
	
    return result;
}

int Camera::ReadPixelformat(uint32_t &pixelformat, uint32_t &bytesPerLine, QString &pfText)
{
    int result = -1;
	v4l2_format fmt;
	
	CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_FMT, &fmt))
	{                
		Logger::LogEx("Camera::ReadPixelformat VIDIOC_G_FMT OK =%d", fmt.fmt.pix.pixelformat);
        emit OnCameraMessage_Signal(QString("ReadPixelformat VIDIOC_G_FMT: OK =%1.").arg(fmt.fmt.pix.pixelformat));

		pixelformat = fmt.fmt.pix.pixelformat;
		bytesPerLine = fmt.fmt.pix.bytesperline;
		pfText = QString(V4l2Helper::ConvertPixelformat2EnumString(fmt.fmt.pix.pixelformat).c_str());
		
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::ReadPixelformat VIDIOC_G_FMT failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("ReadPixelformat VIDIOC_G_FMT: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
	
    return result;
}

int Camera::ReadGain(uint32_t &gain)
{
    int result = -1;
	v4l2_queryctrl ctrl;
	
	CLEAR(ctrl);
	ctrl.id = V4L2_CID_GAIN;
	
	if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCTRL, &ctrl) >= 0)
	{
		v4l2_control fmt;

		Logger::LogEx("Camera::ReadGain VIDIOC_QUERYCTRL V4L2_CID_GAIN OK");
		emit OnCameraMessage_Signal(QString("ReadGain VIDIOC_QUERYCTRL V4L2_CID_GAIN: OK."));

		CLEAR(fmt);
		fmt.id = V4L2_CID_GAIN;

		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_CTRL, &fmt))
		{       
			Logger::LogEx("Camera::ReadGain VIDIOC_G_CTRL V4L2_CID_GAIN OK =%d", fmt.value);
			emit OnCameraMessage_Signal(QString("ReadGain VIDIOC_G_CTRL: V4L2_CID_GAIN OK =%1.").arg(fmt.value));
         
			gain = fmt.value;
			
			result = 0;
		}
		else
		{
			Logger::LogEx("Camera::ReadGain VIDIOC_G_CTRL V4L2_CID_GAIN failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
			emit OnCameraError_Signal(QString("ReadGain VIDIOC_G_CTRL: V4L2_CID_GAIN failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		}
	}
	else
	{
		Logger::LogEx("Camera::ReadGain VIDIOC_QUERYCTRL V4L2_CID_GAIN failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
		emit OnCameraMessage_Signal(QString("ReadGain VIDIOC_QUERYCTRL V4L2_CID_GAIN: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
		result = -2;
	}
	
    return result;
}

int Camera::SetGain(uint32_t gain)
{
	int result = -1;
	v4l2_control fmt;
	
	CLEAR(fmt);
    fmt.id = V4L2_CID_GAIN;
	fmt.value = gain;

	if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_CTRL, &fmt))
	{                
		Logger::LogEx("Camera::SetGain VIDIOC_S_CTRL V4L2_CID_GAIN to %d OK", gain);
        emit OnCameraMessage_Signal(QString("SetGain VIDIOC_S_CTRL: V4L2_CID_GAIN to %1 OK.").arg(gain));

		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::SetGain VIDIOC_S_CTRL V4L2_CID_GAIN failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetGain VIDIOC_S_CTRL: V4L2_CID_GAIN failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
	
    return result;
}

int Camera::ReadAutoGain(bool &autogain)
{
    int result = -1;
	v4l2_queryctrl ctrl;
	
	CLEAR(ctrl);
	ctrl.id = V4L2_CID_AUTOGAIN;
	
	if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCTRL, &ctrl) >= 0)
	{
		v4l2_control fmt;

		Logger::LogEx("Camera::ReadAutoGain VIDIOC_QUERYCTRL V4L2_CID_AUTOGAIN OK");
		emit OnCameraMessage_Signal(QString("ReadAutoGain VIDIOC_QUERYCTRL: V4L2_CID_AUTOGAIN OK."));
		
		CLEAR(fmt);
		fmt.id = V4L2_CID_AUTOGAIN;

		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_CTRL, &fmt))
		{       
			Logger::LogEx("Camera::ReadAutoGain VIDIOC_G_CTRL V4L2_CID_AUTOGAIN OK =%d", fmt.value);
			emit OnCameraMessage_Signal(QString("ReadAutoGain VIDIOC_G_CTRL: V4L2_CID_AUTOGAIN OK =%1.").arg(fmt.value));
         
			autogain = fmt.value;
			
			result = 0;
		}
		else
		{
			Logger::LogEx("Camera::ReadAutoGain VIDIOC_G_CTRL V4L2_CID_AUTOGAIN failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
			emit OnCameraError_Signal(QString("ReadAutoGain VIDIOC_G_CTRL: V4L2_CID_AUTOGAIN failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		}
	}
	else
	{
		Logger::LogEx("Camera::ReadAutoGain VIDIOC_QUERYCTRL V4L2_CID_AUTOGAIN failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
		emit OnCameraMessage_Signal(QString("ReadAutoGain VIDIOC_QUERYCTRL: V4L2_CID_AUTOGAIN failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
		result = -2;
	}
	
    return result;
}

int Camera::SetAutoGain(bool autogain)
{
    int result = -1;
	v4l2_control fmt;
	
	CLEAR(fmt);
    fmt.id = V4L2_CID_AUTOGAIN;
	fmt.value = autogain;

	if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_CTRL, &fmt))
	{          
		Logger::LogEx("Camera::SetAutoGain VIDIOC_S_CTRL V4L2_CID_AUTOGAIN to %s OK", ((autogain)?"true":"false"));
        emit OnCameraMessage_Signal(QString("SetAutoGain VIDIOC_S_CTRL: V4L2_CID_AUTOGAIN to %1 OK.").arg((autogain)?"true":"false"));
		
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::SetAutoGain VIDIOC_S_CTRL V4L2_CID_AUTOGAIN failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetAutoGain VIDIOC_S_CTRL: V4L2_CID_AUTOGAIN failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
	
    return result;
}

int Camera::ReadExposure(uint32_t &exposure)
{
    int result = -1;
	v4l2_queryctrl ctrl;
	
	CLEAR(ctrl);
	ctrl.id = V4L2_CID_EXPOSURE;
	
	if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCTRL, &ctrl) >= 0)
	{
		v4l2_control fmt;

		Logger::LogEx("Camera::ReadExposure VIDIOC_QUERYCTRL Enum V4L2_CID_EXPOSURE OK");
		emit OnCameraMessage_Signal(QString("ReadExposure VIDIOC_QUERYCTRL Enum V4L2_CID_EXPOSURE: OK."));
		
		CLEAR(fmt);
		fmt.id = V4L2_CID_EXPOSURE;

		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_CTRL, &fmt))
		{                
			Logger::LogEx("Camera::ReadExposure VIDIOC_G_CTRL V4L2_CID_EXPOSURE OK =%d", fmt.value);
			emit OnCameraMessage_Signal(QString("ReadExposure VIDIOC_G_CTRL: V4L2_CID_EXPOSURE OK =%1.").arg(fmt.value));

			exposure = fmt.value;
			
			result = 0;
		}
		else
		{
			Logger::LogEx("Camera::ReadExposure VIDIOC_G_CTRL V4L2_CID_EXPOSURE failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
			emit OnCameraError_Signal(QString("ReadExposure VIDIOC_G_CTRL: V4L2_CID_EXPOSURE failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		}
	}
	else
	{
		Logger::LogEx("Camera::ReadExposure VIDIOC_QUERYCTRL Enum V4L2_CID_EXPOSURE failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
		emit OnCameraMessage_Signal(QString("ReadExposure VIDIOC_QUERYCTRL Enum V4L2_CID_EXPOSURE: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
		result = -2;
	}
	
    return result;
}

int Camera::ReadExposureAbs(uint32_t &exposure)
{
    int result = -1;
    v4l2_queryctrl ctrl;
    
    CLEAR(ctrl);
    ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    
    if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCTRL, &ctrl) >= 0)
    {
        v4l2_control fmt;

        Logger::LogEx("Camera::ReadExposureAbs VIDIOC_QUERYCTRL Enum V4L2_CID_EXPOSURE_ABSOLUTE OK");
        emit OnCameraMessage_Signal(QString("ReadExposureAbs VIDIOC_QUERYCTRL Enum V4L2_CID_EXPOSURE_ABSOLUTE: OK."));
        
        CLEAR(fmt);
        fmt.id = V4L2_CID_EXPOSURE_ABSOLUTE;

        if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_CTRL, &fmt))
        {                
            Logger::LogEx("Camera::ReadExposureAbs VIDIOC_G_CTRL V4L2_CID_EXPOSURE_ABSOLUTE OK =%d", fmt.value);
            emit OnCameraMessage_Signal(QString("ReadExposureAbs VIDIOC_G_CTRL: V4L2_CID_EXPOSURE_ABSOLUTE OK =%1.").arg(fmt.value));

            exposure = fmt.value;
            
            result = 0;
        }
        else
        {
            Logger::LogEx("Camera::ReadExposureAbs VIDIOC_G_CTRL V4L2_CID_EXPOSURE_ABSOLUTE failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
            emit OnCameraError_Signal(QString("ReadExposureAbs VIDIOC_G_CTRL: V4L2_CID_EXPOSURE_ABSOLUTE failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
        }
    }
    else
    {
        Logger::LogEx("Camera::ReadExposureAbs VIDIOC_QUERYCTRL Enum V4L2_CID_EXPOSURE_ABSOLUTE failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraMessage_Signal(QString("ReadExposureAbs VIDIOC_QUERYCTRL Enum V4L2_CID_EXPOSURE_ABSOLUTE: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
        
        result = -2;
    }
    
    return result;
}

int Camera::SetExposure(uint32_t exposure)
{
    int result = -1;
	v4l2_control fmt;
	
	CLEAR(fmt);
    fmt.id = V4L2_CID_EXPOSURE;
	fmt.value = exposure;

	if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_CTRL, &fmt))
	{                
		Logger::LogEx("Camera::SetExposure VIDIOC_S_CTRL V4L2_CID_EXPOSURE to %d OK", exposure);
        emit OnCameraMessage_Signal(QString("SetExposure VIDIOC_S_CTRL: V4L2_CID_EXPOSURE to %1 OK.").arg(exposure));
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::SetExposure VIDIOC_S_CTRL V4L2_CID_EXPOSURE failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetExposure VIDIOC_S_CTRL: V4L2_CID_EXPOSURE failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
	
    return result;
}

int Camera::SetExposureAbs(uint32_t exposure)
{
    int result = -1;
    v4l2_control fmt;
    
    CLEAR(fmt);
    fmt.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    fmt.value = exposure;

    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_CTRL, &fmt))
    {                
        Logger::LogEx("Camera::SetExposureAbs VIDIOC_S_CTRL V4L2_CID_EXPOSURE_ABSOLUTE to %d OK", exposure);
        emit OnCameraMessage_Signal(QString("SetExposureAbs VIDIOC_S_CTRL: V4L2_CID_EXPOSURE_ABSOLUTE to %1 OK.").arg(exposure));
        result = 0;
    }
    else
    {
        Logger::LogEx("Camera::SetExposureAbs VIDIOC_S_CTRL V4L2_CID_EXPOSURE_ABSOLUTE failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetExposureAbs VIDIOC_S_CTRL: V4L2_CID_EXPOSURE_ABSOLUTE failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
    }
    
    return result;
}

int Camera::ReadAutoExposure(bool &autoexposure)
{
    int result = -1;
	v4l2_queryctrl ctrl;
	
	CLEAR(ctrl);
	ctrl.id = V4L2_CID_EXPOSURE_AUTO;
	
	if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCTRL, &ctrl) >= 0)
	{
		v4l2_control fmt;
		
		Logger::LogEx("Camera::ReadAutoExposure VIDIOC_QUERYCTRL V4L2_CID_EXPOSURE_AUTO OK");
		emit OnCameraMessage_Signal(QString("ReadAutoExposure VIDIOC_QUERYCTRL: V4L2_CID_EXPOSURE_AUTO OK."));

		CLEAR(fmt);
		fmt.id = V4L2_CID_EXPOSURE_AUTO;

		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_CTRL, &fmt))
		{                
			Logger::LogEx("Camera::ReadAutoExposure VIDIOC_G_CTRL V4L2_CID_EXPOSURE_AUTO OK =%d", fmt.value);
			emit OnCameraMessage_Signal(QString("ReadAutoExposure VIDIOC_G_CTRL: V4L2_CID_EXPOSURE_AUTO OK =%1.").arg(fmt.value));

			autoexposure = fmt.value;
			
			result = 0;
		}
		else
		{
			Logger::LogEx("Camera::ReadAutoExposure VIDIOC_G_CTRL V4L2_CID_EXPOSURE_AUTO failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
			emit OnCameraError_Signal(QString("ReadAutoExposure VIDIOC_G_CTRL: V4L2_CID_EXPOSURE_AUTO failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		}
	}
	else
	{
		Logger::LogEx("Camera::ReadAutoExposure VIDIOC_QUERYCTRL Enum V4L2_CID_EXPOSURE_AUTO failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
		emit OnCameraMessage_Signal(QString("ReadAutoExposure VIDIOC_QUERYCTRL Enum V4L2_CID_EXPOSURE_AUTO: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
		result = -2;
	}
	
    return result;
}

int Camera::SetAutoExposure(bool autoexposure)
{
    int result = -1;
	v4l2_control fmt;
	
	CLEAR(fmt);
    fmt.id = V4L2_CID_EXPOSURE_AUTO;
	fmt.value = autoexposure;

	if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_CTRL, &fmt))
	{                
		Logger::LogEx("Camera::SetAutoExposure VIDIOC_S_CTRL V4L2_CID_EXPOSURE_AUTO to %s OK", ((autoexposure)?"true":"false"));
        emit OnCameraMessage_Signal(QString("SetAutoExposure VIDIOC_S_CTRL: V4L2_CID_EXPOSURE_AUTO to %1 OK.").arg(((autoexposure)?"true":"false")));
		
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::SetAutoExposure VIDIOC_S_CTRL V4L2_CID_EXPOSURE_AUTO failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetAutoExposure VIDIOC_S_CTRL: V4L2_CID_EXPOSURE_AUTO failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
	
    return result;
}

int Camera::ReadControl(uint32_t &value, uint32_t controlID, const char *functionName, const char* controlName)
{
    int result = -1;
    v4l2_queryctrl ctrl;
	
    CLEAR(ctrl);
    ctrl.id = controlID;
	
    if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCTRL, &ctrl) >= 0)
    {
	v4l2_control fmt;
	
	Logger::LogEx("Camera::%s VIDIOC_QUERYCTRL %s OK", functionName, controlName);
	emit OnCameraMessage_Signal(QString("%1 VIDIOC_QUERYCTRL: %2 OK.").arg(functionName).arg(controlName));

	CLEAR(fmt);
	fmt.id = controlID;

	if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_CTRL, &fmt))
	{                
	    Logger::LogEx("Camera::%s VIDIOC_G_CTRL %s OK =%d", functionName, controlName, fmt.value);
	    emit OnCameraMessage_Signal(QString("%1 VIDIOC_G_CTRL: %2 OK =%3.").arg(functionName).arg(controlName).arg(fmt.value));

	    value = fmt.value;
			
	    result = 0;
	}
	else
	{
	    Logger::LogEx("Camera::%s VIDIOC_G_CTRL %s failed errno=%d=%s", functionName, controlName, errno, V4l2Helper::ConvertErrno2String(errno).c_str());
	    emit OnCameraError_Signal(QString("%1 VIDIOC_G_CTRL: %2 failed errno=%3=%4.").arg(functionName).arg(controlName).arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
    }
    else
    {
	Logger::LogEx("Camera::%s VIDIOC_QUERYCTRL Enum %s failed errno=%d=%s", functionName, controlName, errno, V4l2Helper::ConvertErrno2String(errno).c_str());
	emit OnCameraMessage_Signal(QString("%1 VIDIOC_QUERYCTRL Enum %2: failed errno=%3=%4.").arg(functionName).arg(controlName).arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
	result = -2;
    }
	
    return result;
}

int Camera::SetControl(uint32_t value, uint32_t controlID, const char *functionName, const char* controlName)
{
    int result = -1;
    v4l2_control fmt;
	
    CLEAR(fmt);
    fmt.id = controlID;
    fmt.value = value;

    if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_CTRL, &fmt))
    {                
	Logger::LogEx("Camera::%s VIDIOC_S_CTRL %s to %d OK", functionName, controlName, value);
        emit OnCameraMessage_Signal(QString("%1 VIDIOC_S_CTRL: %2 to %3 OK.").arg(functionName).arg(controlName).arg(value));
	result = 0;
    }
    else
    {
	Logger::LogEx("Camera::%s VIDIOC_S_CTRL %s failed errno=%d=%s", functionName, controlName, errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("%1 VIDIOC_S_CTRL: %2 failed errno=%3=%4.").arg(functionName).arg(controlName).arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
    }
	
    return result;
}

int Camera::ReadGamma(uint32_t &value)
{
    return ReadControl(value, V4L2_CID_GAMMA, "ReadGamma", "V4L2_CID_GAMMA");
}

int Camera::SetGamma(uint32_t value)
{
    return SetControl(value, V4L2_CID_GAMMA, "SetGamma", "V4L2_CID_GAMMA");
}

int Camera::ReadReverseX(uint32_t &value)
{
    return ReadControl(value, V4L2_CID_HFLIP, "ReadReverseX", "V4L2_CID_HFLIP");
}

int Camera::SetReverseX(uint32_t value)
{
    return SetControl(value, V4L2_CID_HFLIP, "SetReverseX", "V4L2_CID_HFLIP");
}

int Camera::ReadReverseY(uint32_t &value)
{
    return ReadControl(value, V4L2_CID_VFLIP, "ReadReverseY", "V4L2_CID_VFLIP");
}

int Camera::SetReverseY(uint32_t value)
{
    return SetControl(value, V4L2_CID_VFLIP, "SetReverseY", "V4L2_CID_VFLIP");
}

int Camera::ReadSharpness(uint32_t &value)
{
    return ReadControl(value, V4L2_CID_SHARPNESS, "ReadSharpness", "V4L2_CID_SHARPNESS");
}

int Camera::SetSharpness(uint32_t value)
{
    return SetControl(value, V4L2_CID_SHARPNESS, "SetSharpness", "V4L2_CID_SHARPNESS");
}

int Camera::ReadBrightness(uint32_t &value)
{
    return ReadControl(value, V4L2_CID_BRIGHTNESS, "ReadBrightness", "V4L2_CID_BRIGHTNESS");
}

int Camera::SetBrightness(uint32_t value)
{
    return SetControl(value, V4L2_CID_BRIGHTNESS, "SetBrightness", "V4L2_CID_BRIGHTNESS");
}

int Camera::ReadContrast(uint32_t &value)
{
    return ReadControl(value, V4L2_CID_CONTRAST, "ReadContrast", "V4L2_CID_CONTRAST");
}

int Camera::SetContrast(uint32_t value)
{
    return SetControl(value, V4L2_CID_CONTRAST, "SetContrast", "V4L2_CID_CONTRAST");
}

int Camera::ReadSaturation(uint32_t &value)
{
    return ReadControl(value, V4L2_CID_SATURATION, "ReadSaturation", "V4L2_CID_SATURATION");
}

int Camera::SetSaturation(uint32_t value)
{
    return SetControl(value, V4L2_CID_SATURATION, "SetSaturation", "V4L2_CID_SATURATION");
}

int Camera::ReadHue(uint32_t &value)
{
    return ReadControl(value, V4L2_CID_HUE, "ReadHue", "V4L2_CID_HUE");
}

int Camera::SetHue(uint32_t value)
{
    return SetControl(value, V4L2_CID_HUE, "SetHue", "V4L2_CID_HUE");
}

int Camera::SetContinousWhiteBalance(bool flag)
{
    if (flag)
	return SetControl(flag, V4L2_CID_AUTO_WHITE_BALANCE, "SetContinousWhiteBalance on", "V4L2_CID_AUTO_WHITE_BALANCE");
    else
        return SetControl(flag, V4L2_CID_AUTO_WHITE_BALANCE, "SetContinousWhiteBalance off", "V4L2_CID_AUTO_WHITE_BALANCE");
}

int Camera::DoWhiteBalanceOnce()
{
    return SetControl(0, V4L2_CID_DO_WHITE_BALANCE, "DoWhiteBalanceOnce", "V4L2_CID_DO_WHITE_BALANCE");
}

int Camera::ReadRedBalance(uint32_t &value)
{
    return ReadControl(value, V4L2_CID_RED_BALANCE, "ReadRedBalance", "V4L2_CID_RED_BALANCE");
}

int Camera::SetRedBalance(uint32_t value)
{
    return SetControl(value, V4L2_CID_RED_BALANCE, "SetRedBalance", "V4L2_CID_RED_BALANCE");
}

int Camera::ReadBlueBalance(uint32_t &value)
{
    return ReadControl(value, V4L2_CID_BLUE_BALANCE, "ReadBlueBalance", "V4L2_CID_BLUE_BALANCE");
}

int Camera::SetBlueBalance(uint32_t value)
{
    return SetControl(value, V4L2_CID_BLUE_BALANCE, "SetBlueBalance", "V4L2_CID_BLUE_BALANCE");
}

int Camera::ReadFramerate(uint32_t &numerator, uint32_t &denominator, uint32_t width, uint32_t height, uint32_t pixelformat)
{
    int result = -1;
    v4l2_streamparm parm;
	
    CLEAR(parm);
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
    if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_PARM, &parm) >= 0 &&
        parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
    {
        v4l2_frmivalenum frmival;
	CLEAR(frmival);
	
	frmival.index = 0;
	frmival.pixel_format = pixelformat;
	frmival.width = width;
	frmival.height = height;
        while (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0)
	{
	    frmival.index++;
	    Logger::LogEx("Camera::ReadFramerate type=%d", frmival.type);
	    emit OnCameraMessage_Signal(QString("ReadFramerate type=%1").arg(frmival.type));
	}
	if (frmival.index == 0)
	{
	    Logger::LogEx("Camera::ReadFramerate VIDIOC_ENUM_FRAMEINTERVALS failed");
	    emit OnCameraMessage_Signal(QString("ReadFramerate VIDIOC_ENUM_FRAMEINTERVALS failed"));
	}
	
	v4l2_streamparm parm;
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
	if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_PARM, &parm) >= 0)
	{
	    numerator = parm.parm.capture.timeperframe.numerator;
	    denominator = parm.parm.capture.timeperframe.denominator;
	    Logger::LogEx("Camera::ReadFramerate %d/%dOK", numerator, denominator);
	    emit OnCameraMessage_Signal(QString("ReadFramerate OK %1/%2").arg(numerator).arg(denominator));
	}
    }
    else
    {
	Logger::LogEx("Camera::ReadFramerate VIDIOC_G_PARM V4L2_CAP_TIMEPERFRAME failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
	emit OnCameraMessage_Signal(QString("ReadFramerate VIDIOC_G_PARM V4L2_CAP_TIMEPERFRAME: failed errno=%3=%4.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
	result = -2;
    }
	
    return result;
}

int Camera::SetFramerate(uint32_t numerator, uint32_t denominator)
{
    int result = -1;
    v4l2_streamparm parm;
	
    CLEAR(parm);
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_PARM, &parm) >= 0)
    {
	parm.parm.capture.timeperframe.numerator = numerator;
	parm.parm.capture.timeperframe.denominator = denominator;
	if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_PARM, &parm))
	{                
	    Logger::LogEx("Camera::SetFramerate VIDIOC_S_PARM to %d/%d OK", numerator, denominator);
	    emit OnCameraMessage_Signal(QString("SetFramerate VIDIOC_S_PARM: %1/%2 OK.").arg(numerator).arg(denominator));
	    result = 0;
	}
	else
	{
	    Logger::LogEx("Camera::SetFramerate VIDIOC_S_PARM failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
	    emit OnCameraError_Signal(QString("SetFramerate VIDIOC_S_PARM: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
    }
    
    return result;
}

int Camera::ReadCropCapabilities(uint32_t &boundsx, uint32_t &boundsy, uint32_t &boundsw, uint32_t &boundsh,
				 uint32_t &defrectx, uint32_t &defrecty, uint32_t &defrectw, uint32_t &defrecth,
				 uint32_t &aspectnum, uint32_t &aspectdenum)
{
    int result = -1;
    v4l2_cropcap cropcap;
	
    CLEAR(cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
    if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_CROPCAP, &cropcap) >= 0)
    {
        boundsx = cropcap.bounds.left;
	boundsy = cropcap.bounds.top;
	boundsw = cropcap.bounds.width;
	boundsh = cropcap.bounds.height;
	defrectx = cropcap.defrect.left;
	defrecty = cropcap.defrect.top;
	defrectw = cropcap.defrect.width;
	defrecth = cropcap.defrect.height;
	aspectnum = cropcap.pixelaspect.numerator;
	aspectdenum = cropcap.pixelaspect.denominator;
	Logger::LogEx("Camera::ReadCrop VIDIOC_CROPCAP bx=%d, by=%d, bw=%d, bh=%d, dx=%d, dy=%d, dw=%d, dh=%d, num=%d, denum=%d OK", 
		      boundsx, boundsy, boundsw, boundsh, defrectx, defrecty, defrectw, defrecth, aspectnum, aspectdenum);
	emit OnCameraMessage_Signal(QString("ReadCrop VIDIOC_CROPCAP OK bx=%1,by=%2, bw=%3, bh=%4, dx=%5, dy=%6, dw=%7, dh=%8, num=%9, denum=%10")
		      .arg(boundsx).arg(boundsy).arg(boundsw).arg(boundsh).arg(defrectx).arg(defrecty).arg(defrectw).arg(defrecth).arg(aspectnum).arg(aspectdenum));	
	
	result = 0;
    }
    else
    {
	Logger::LogEx("Camera::ReadCrop VIDIOC_CROPCAP failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
	emit OnCameraMessage_Signal(QString("ReadCrop VIDIOC_CROPCAP: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
	result = -2;
    }
	
    return result;
}

int Camera::ReadCrop(uint32_t &xOffset, uint32_t &yOffset, uint32_t &width, uint32_t &height)
{
    int result = -1;
    v4l2_crop crop;
	
    CLEAR(crop);
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
    if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_CROP, &crop) >= 0)
    {
        xOffset = crop.c.left;
	yOffset = crop.c.top;
	width = crop.c.width;
	height = crop.c.height;
	Logger::LogEx("Camera::ReadCrop VIDIOC_G_CROP x=%d, y=%d, w=%d, h=%d OK", xOffset, yOffset, width, height);
	emit OnCameraMessage_Signal(QString("ReadCrop VIDIOC_G_CROP OK %1,%2, %3, %4").arg(xOffset).arg(yOffset).arg(width).arg(height));	
	
	result = 0;
    }
    else
    {
	Logger::LogEx("Camera::ReadCrop VIDIOC_G_CROP failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
	emit OnCameraMessage_Signal(QString("ReadCrop VIDIOC_G_CROP: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
	result = -2;
    }
	
    return result;
}

int Camera::SetCrop(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height)
{
    int result = -1;
    v4l2_crop crop;
    
    CLEAR(crop);
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c.left = xOffset;
    crop.c.top = yOffset;
    crop.c.width = width;
    crop.c.height = height;
	
    if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_CROP, &crop) >= 0)
    {
	Logger::LogEx("Camera::SetCrop VIDIOC_S_CROP %d, %d, %d, %d OK", xOffset, yOffset, width, height);
	emit OnCameraMessage_Signal(QString("SetCrop VIDIOC_S_CROP %1, %2, %3, %4 OK ").arg(xOffset).arg(yOffset).arg(width).arg(height));	
	
	result = 0;
    }
    else
    {
	Logger::LogEx("Camera::SetCrop VIDIOC_S_CROP failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
	emit OnCameraMessage_Signal(QString("SetCrop VIDIOC_S_CROP: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
    }
  	
    return result;
}

/*********************************************************************************************************/
// Frame buffer handling
/*********************************************************************************************************/

int Camera::CreateUserBuffer(uint32_t bufferCount, uint32_t bufferSize)
{
    int result = -1;

    result = m_StreamCallbacks->CreateAllUserBuffer(bufferCount, bufferSize);

    return result;
}

int Camera::QueueAllUserBuffer()
{
    int result = -1;
	
    result = m_StreamCallbacks->QueueAllUserBuffer();
    
    return result;
}

int Camera::QueueSingleUserBuffer(const int index)
{
    int result = 0;

    result = m_StreamCallbacks->QueueSingleUserBuffer(index);
    
    return result;
}

int Camera::DeleteUserBuffer()
{
    int result = 0;

    result = m_StreamCallbacks->DeleteAllUserBuffer();
	
    return result;
}

/*********************************************************************************************************/
// Info
/*********************************************************************************************************/

int Camera::GetCameraDriverName(uint32_t index, std::string &strText)
{
    int result = 0;
	
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	// queuery device capabilities
	if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCAP, &cap)) 
	{
        Logger::LogEx("Camera::GetCameraDriverName VIDIOC_QUERYCAP %s is no V4L2 device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraDriverName VIDIOC_QUERYCAP " + QString(m_DeviceName.c_str()) + " is no V4L2 device.");
        return -1;
    } 
	else
	{
		Logger::LogEx("Camera::GetCameraDriverName VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
	}

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
        Logger::LogEx("Camera::GetCameraDriverName %s is no video capture device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraDriverName " + QString(m_DeviceName.c_str()) + " is no video capture device.");
        return -1;
    }
	else
	{
		Logger::LogEx("Camera::GetCameraDriverName VIDIOC_QUERYCAP %s driver name=%s\n", m_DeviceName.c_str(), (char*)cap.driver);
	}
	
    strText = (char*)cap.driver;
    
    return result;
}

int Camera::GetCameraDeviceName(uint32_t index, std::string &strText)
{
    int result = 0;
	
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	// queuery device capabilities
	if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCAP, &cap)) 
	{
        Logger::LogEx("Camera::GetCameraDeviceName %s is no V4L2 device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraDeviceName " + QString(m_DeviceName.c_str()) + " is no V4L2 device.");
        return -1;
    }
	else
	{
		Logger::LogEx("Camera::GetCameraDeviceName VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
	}

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
        Logger::LogEx("Camera::GetCameraDeviceName %s is no video capture device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraDeviceName " + QString(m_DeviceName.c_str()) + " is no video capture device.");
        return -1;
    }
	else
	{
		Logger::LogEx("Camera::GetCameraDeviceName VIDIOC_QUERYCAP %s device name=%s\n", m_DeviceName.c_str(), (char*)cap.card);
	}

    strText = (char*)cap.card;

    return result;
}

int Camera::GetCameraBusInfo(uint32_t index, std::string &strText)
{
    int result = 0;
	
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	// queuery device capabilities
	if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCAP, &cap)) 
	{
        Logger::LogEx("Camera::GetCameraBusInfo %s is no V4L2 device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraBusInfo " + QString(m_DeviceName.c_str()) + " is no V4L2 device.");
        return -1;
    }
	else
	{
		Logger::LogEx("Camera::GetCameraBusInfo VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
	}

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
        Logger::LogEx("Camera::GetCameraBusInfo %s is no video capture device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraBusInfo " + QString(m_DeviceName.c_str()) + " is no video capture device.");
        return -1;
    }
	else
	{
		Logger::LogEx("Camera::GetCameraBusInfo VIDIOC_QUERYCAP %s bus info=%s\n", m_DeviceName.c_str(), (char*)cap.bus_info);
	}

    strText = (char*)cap.bus_info;

    return result;
}

int Camera::GetCameraDriverVersion(uint32_t index, std::string &strText)
{
    int result = 0;
	std::stringstream tmp;
	
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	// queuery device capabilities
	if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCAP, &cap)) 
	{
        Logger::LogEx("Camera::GetCameraDriverVersion %s is no V4L2 device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraDriverVersion " + QString(m_DeviceName.c_str()) + " is no V4L2 device.");
        return -1;
    }
	else
	{
		Logger::LogEx("Camera::GetCameraDriverVersion VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
	}

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
        Logger::LogEx("Camera::GetCameraDriverVersion %s is no video capture device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraDriverVersion " + QString(m_DeviceName.c_str()) + " is no video capture device.");
        return -1;
    }
	else
	{
		tmp << ((cap.version/0x10000)&0xFF) << "." << ((cap.version/0x100)&0xFF) << "." << (cap.version&0xFF);
		Logger::LogEx("Camera::GetCameraDriverVersion VIDIOC_QUERYCAP %s driver version=%s\n", m_DeviceName.c_str(), tmp.str().c_str());
	}

    strText = tmp.str();
    
    return result;
}

int Camera::GetCameraCapabilities(uint32_t index, std::string &strText)
{
	int result = 0;
	std::stringstream tmp;
	
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	// queuery device capabilities
	if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCAP, &cap)) 
	{
        Logger::LogEx("Camera::GetCameraCapabilities %s is no V4L2 device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraCapabilities " + QString(m_DeviceName.c_str()) + " is no V4L2 device.");
        return -1;
    }
	else
	{
		Logger::LogEx("Camera::GetCameraCapabilities VIDIOC_QUERYCAP %s OK\n", m_DeviceName.c_str());
	}

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
        Logger::LogEx("Camera::GetCameraCapabilities %s is no video capture device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraCapabilities " + QString(m_DeviceName.c_str()) + " is no video capture device.");
        return -1;
    }
	else
	{
		tmp << "0x" << std::hex << cap.capabilities << std::endl
			<< "    Read/Write = " << ((cap.capabilities & V4L2_CAP_READWRITE)?"Yes":"No") << std::endl
			<< "    Streaming = " << ((cap.capabilities & V4L2_CAP_STREAMING)?"Yes":"No");
		Logger::LogEx("Camera::GetCameraCapabilities VIDIOC_QUERYCAP %s driver version=%s\n", m_DeviceName.c_str(), tmp.str().c_str());
	}

	
    strText = tmp.str();
    
    return result;
}

    
// Recording
void Camera::SetRecording(bool start)
{
    m_StreamCallbacks->SetRecording(start);
}

void Camera::DisplayStepBack()
{
    m_StreamCallbacks->DisplayStepBack();
}

void Camera::DisplayStepForw()
{
    m_StreamCallbacks->DisplayStepForw();
}

void Camera::DeleteRecording()
{
    m_StreamCallbacks->DeleteRecording();
}

/*********************************************************************************************************/
// Commands
/*********************************************************************************************************/

void Camera::SwitchFrameTransfer2GUI(bool showFrames)
{
    if (m_StreamCallbacks != 0)
        m_StreamCallbacks->SwitchFrameTransfer2GUI(showFrames);

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

int Camera::ReadCSVFile(const char *pFilename, std::vector<uint8_t> &rData)
{
    if (NULL == pFilename)
	return -1;
    
    // read csv file
    FILE *pFile = fopen(pFilename, "rt");
    if (NULL == pFile)
	return -2;
    
    size_t nSize = fsize(pFilename);
    std::string fileText(nSize, '\0');
    size_t readCount = fread(&(fileText[0]), 1, nSize, pFile);
    if (readCount != nSize)
    {
	fclose(pFile);
	return -3;
    }

    fclose(pFile);

    emit OnCameraMessage_Signal("ReadCSVFile: " + QString(pFilename) + " successfully loaded.");
    
    // check width and height of csv data
    uint32_t width = 0;
    uint32_t height = 0;
    
    QString tmp = fileText.c_str();
    QStringList lines = tmp.split("\r\n");
    
    for (int i=0; i<lines.size(); i++)
    {
	QString line = lines.at(i);
	QStringList items = line.split(',');
	if (items.size() >= 10)
	{
	    QString byte = items.at(6);
	    uint8_t dataID = byte.toUInt(0, 16);
	    
	    if (dataID > 0x12)
	    {
		QString byte = items.at(8);
		uint32_t length = (((uint8_t)items.at(8).toUInt(0, 16)) << 8);
		byte = items.at(7);
		length += items.at(7).toUInt(0, 16);
	    
		if (length > 0 && width == 0)
		{
		    width = length;
		}
	    
		if (length == width)
		{
		    height++;
		}
	    }
	}
    }
    
    emit OnCameraMessage_Signal(QString("ReadCSVFile: %1 data per row and %2 lines detected.").arg(width).arg(height));
    
    // transfer data from input String to data
    rData.resize(width * height);
    
    uint32_t counter = 0;
    for (int i=0; i<lines.size(); i++)
    {
	QString line = lines.at(i);
	QStringList items = line.split(',');
	if (items.size() >= 10)
	{
	    QString byte = items.at(6);
	    uint8_t dataID = byte.toUInt(0, 16);
	    
	    if (dataID > 0x12)
	    {
		QString byte = items.at(8);
		uint32_t length = (((uint8_t)items.at(8).toUInt(0, 16)) << 8);
		byte = items.at(7);
		length += items.at(7).toUInt(0, 16);
	    
		if (length > 0)
		{
		    const uint32_t lineHeader = 10;
		    for (int x=0; x<length; x++)
		    {
			rData[counter++] =  items.at(lineHeader + x).toUInt(0, 16);
		    }
		}
	    }
	}
    }
    
    emit OnCameraMessage_Signal(QString("ReadCSVFile: data cache filled with %1 bytes.").arg(width*height));
}


}}} // namespace AVT::Tools::Examples

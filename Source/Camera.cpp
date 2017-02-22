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
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "Helper.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

namespace AVT {
namespace Tools {
namespace Examples {

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////    

Camera::Camera()
    : m_bSIRunning(false)
    , m_UsedBufferCount(0)
    , m_nFileDescriptor(-1)
{
    connect(&m_DmaDeviceDiscoveryCallbacks, SIGNAL(OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &)), this, SLOT(OnCameraListChanged(const int &, unsigned int, unsigned long long, const QString &)));

    connect(&m_DmaSICallbacks, SIGNAL(OnFrameReady_Signal(const QImage &, const unsigned long long &)), this, SLOT(OnFrameReady(const QImage &, const unsigned long long &)));
	connect(&m_DmaSICallbacks, SIGNAL(OnFrameDone_Signal(const unsigned long long)), this, SLOT(OnFrameDone(const unsigned long long)));
    connect(&m_DmaSICallbacks, SIGNAL(OnRecordFrame_Signal(const unsigned long long &, const unsigned long long &)), this, SLOT(OnRecordFrame(const unsigned long long &, const unsigned long long &)));
    connect(&m_DmaSICallbacks, SIGNAL(OnDisplayFrame_Signal(const unsigned long long &, const unsigned long &, const unsigned long &, const unsigned long &)), this, SLOT(OnDisplayFrame(const unsigned long long &, const unsigned long &, const unsigned long &, const unsigned long &)));
    connect(&m_DmaSICallbacks, SIGNAL(OnMessage_Signal(const QString &)), this, SLOT(OnMessage(const QString &)));

}

Camera::~Camera()
{
    m_DmaDeviceDiscoveryCallbacks.SetTerminateFlag();
    m_DmaSICallbacks.SetTerminateFlag();

    CloseDevice();
}

unsigned int Camera::GetReceivedFramesCount()
{
    return m_DmaSICallbacks.GetReceivedFramesCount();
}

unsigned int Camera::GetIncompletedFramesCount()
{
    return m_DmaSICallbacks.GetIncompletedFramesCount();
}

int Camera::OpenDevice(std::string &deviceName)
{
	int result = -1;
	
	if (-1 == m_nFileDescriptor)
	{
		if ((m_nFileDescriptor = open(deviceName.c_str(), O_RDWR | O_NONBLOCK, 0)) == -1) 
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

// The event handler to return the frame
void Camera::OnFrameDone(const unsigned long long frameHandle)
{
    if (m_bSIRunning)
    {
		for (int i = 0; i < m_UsedBufferCount; ++i)
		{
			if (frameHandle == (uint64_t)m_UserBufferContainerList[i]->pBuffer)
			{
				v4l2_buffer buf;

				CLEAR(buf);
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_USERPTR;
				buf.index = i;
				buf.m.userptr = (unsigned long)m_UserBufferContainerList[i]->pBuffer;
				buf.length = m_UserBufferContainerList[i]->nBufferlength;

				V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf);
				
				break;
			}
		}
        		
        
    }
}

// Event will be called when the a frame is recorded
void Camera::OnRecordFrame(const unsigned long long &frameID, const unsigned long long &framesInQueue)
{
    emit OnCameraRecordFrame_Signal(frameID, framesInQueue);
}

// Event will be called when the a frame is displayed
void Camera::OnDisplayFrame(const unsigned long long &frameID, const unsigned long &width, const unsigned long &height, const unsigned long &pixelformat)
{
    emit OnCameraDisplayFrame_Signal(frameID, width, height, pixelformat);
}

// Event will be called when for text notification
void Camera::OnMessage(const QString &msg)
{
    emit OnCameraMessage_Signal(msg);
}

// The event handler to set or remove devices 
void Camera::OnCameraListChanged(const int &reason, unsigned int cardNumber, unsigned long long deviceID, const QString &deviceName)
{
    emit OnCameraListChanged_Signal(reason, cardNumber, deviceID, deviceName);
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
			Logger::LogEx("Camera::DeviceDiscoveryStart open %s failed", deviceName.toAscii().data());
			emit OnCameraError_Signal("DeviceDiscoveryStart: open " + deviceName + " failed");
		}
		else
		{
			struct v4l2_capability cap;
						
			Logger::LogEx("Camera::DeviceDiscoveryStart open %s found", deviceName.toAscii().data());
			emit OnCameraMessage_Signal("DeviceDiscoveryStart: open " + deviceName + " found");
			
			// queuery device capabilities
			if (-1 == V4l2Helper::xioctl(fileDiscriptor, VIDIOC_QUERYCAP, &cap)) 
			{
				Logger::LogEx("Camera::DeviceDiscoveryStart %s is no V4L2 device", deviceName.toAscii().data());
				emit OnCameraError_Signal("DeviceDiscoveryStart: " + deviceName + " is no V4L2 device");
			}
			else
			{
				if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
				{
					Logger::LogEx("Camera::DeviceDiscoveryStart %s is no video capture device", deviceName.toAscii().data());
					emit OnCameraError_Signal("DeviceDiscoveryStart: " + deviceName + " is no video capture device");
				}
			}
			
			
			if (-1 == close(fileDiscriptor))
			{
				Logger::LogEx("Camera::DeviceDiscoveryStart close %s failed", deviceName.toAscii().data());
				emit OnCameraError_Signal("DeviceDiscoveryStart: close " + deviceName + " failed");
			}
			else
			{
				emit OnCameraListChanged_Signal(UpdateTriggerPluggedIn, 0, device_count, deviceName);
			
				device_count++;
			
				Logger::LogEx("Camera::DeviceDiscoveryStart close %s OK", deviceName.toAscii().data());
				emit OnCameraMessage_Signal("DeviceDiscoveryStart: close " + deviceName + " OK");
			}
		}
	}
	while(fileDiscriptor != -1);
	
    //m_DmaDeviceDiscoveryCallbacks.Start();

    return 0;
}

int Camera::DeviceDiscoveryStop()
{
    Logger::LogEx("Device Discovery stopped.");

    //m_DmaDeviceDiscoveryCallbacks.Stop();

    return 0;
}
    
/********************************************************************************/
// SI helper
/********************************************************************************/


int Camera::SIStartChannel(uint32_t pixelformat, uint32_t payloadsize, uint32_t width, uint32_t height, void *pPrivateData)
{
    int nResult = 0;
    
    Logger::LogEx("StartDMA channel , ENDPOINT_MODE_SI.");
	
	m_DmaSICallbacks.SetParameter(m_nFileDescriptor, pixelformat, payloadsize, width, height);

    m_DmaSICallbacks.ResetIncompletedFramesCount();
	
	m_bSIRunning = true;

    /*
    nResult = libcsi_start_stream(m_DeviceHandle, 0x31, 0, 4);
    if (nResult != LIBCSI_ERR_SUCCESS)
    {
        Logger::LogEx("Camera::SIStartChannel libcsi_start_stream devicehandle=%p failed, result=%d=%s", m_DeviceHandle, nResult, ConvertResult2String(nResult).c_str());

        nResult = libcsi_stop_stream(m_DeviceHandle, 1000);
        if (nResult != LIBCSI_ERR_SUCCESS)
            Logger::LogEx("Camera::SIStartChannel libcsi_stop_stream devicehandle=%p failed, result=%d=%s", m_DeviceHandle, nResult, ConvertResult2String(nResult).c_str());
        else
            Logger::LogEx("Camera::SIStartChannel libcsi_stop_stream devicehandle=%p OK", m_DeviceHandle);
    }
    else
    {
        Logger::LogEx("Camera::SIStartChannel libcsi_start_stream devicehandle=%p OK", m_DeviceHandle);
        m_bSIRunning = true;
    }
    */

    return nResult;
}

int Camera::SIStopChannel()
{
    int nResult = 0;
    
    /*
    nResult = libcsi_stop_stream(m_DeviceHandle, 1000);
    if (nResult != 0)
        Logger::LogEx("Camera::SIStopChannel libcsi_stop_stream devicehandle=%p failed, result=%d=%s", m_DeviceHandle, nResult, ConvertResult2String(nResult).c_str());
    else
        Logger::LogEx("Camera::SIStopChannel libcsi_stop_stream devicehandle=%p OK", m_DeviceHandle);
    */

    m_bSIRunning = false;

    /*
    nResult = libcsi_reset_stream_statistics(m_DeviceHandle);
    if (nResult != 0)
        Logger::LogEx("Camera::SIStopChannel libcsi_reset_stream_statistics devicehandle=%p failed, result=%d=%s", m_DeviceHandle, nResult, ConvertResult2String(nResult).c_str());
    else
        Logger::LogEx("Camera::SIStopChannel libcsi_reset_stream_statistics devicehandle=%p OK", m_DeviceHandle);
    */

    Logger::LogEx("StopDMA ENDPOINT_MODE_SI.");
    
    return nResult;
}

/*********************************************************************************************************/
// DCI commands
/*********************************************************************************************************/

int Camera::SendAcquisitionStart()
{
	int result = -1;
	v4l2_buf_type type;
	
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_STREAMON, &type))
	{
        Logger::LogEx("Camera::SendAcquisitionStart Stream ON failed");
        emit OnCameraError_Signal("SendAcquisitionStart: Stream ON failed.");
	}
	else
	{
		result = 0;
	}
	
	return result;
}

int Camera::SendAcquisitionStop()
{
	int result = -1;
	v4l2_buf_type type;
	
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_STREAMOFF, &type))
	{
        Logger::LogEx("Camera::SendAcquisitionStop Stream OFF failed");
        emit OnCameraError_Signal("SendAcquisitionStop: Stream OFF failed.");
	}
	else
	{
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
		payloadsize = fmt.fmt.pix.sizeimage;
		
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::ReadPayloadsize failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("ReadPayloadsize: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
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
		width = fmt.fmt.pix.width;
		height = fmt.fmt.pix.height;
		
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::ReadFrameSize failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("ReadFrameSize: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
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
		fmt.fmt.pix.width = width;
		fmt.fmt.pix.height = height;
	
		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_FMT, &fmt))
		{                
			result = 0;
		}
	}
	else
	{
		Logger::LogEx("Camera::SetFrameSize failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetFrameSize: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
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
		fmt.fmt.pix.width = width;
	
		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_FMT, &fmt))
		{                
			result = 0;
		}
	}
	else
	{
		Logger::LogEx("Camera::SetWidth failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetWidth: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
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
		width = fmt.fmt.pix.width;
		
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::ReadWidth failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("ReadWidth: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
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
		fmt.fmt.pix.height = height;
	
		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_FMT, &fmt))
		{                
			result = 0;
		}
	}
	else
	{
		Logger::LogEx("Camera::SetHeight failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetHeight: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
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
		height = fmt.fmt.pix.height;
		
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::ReadHeight failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("ReadHeight: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
	
    return result;
}

int Camera::ReadFormats()
{
    int result = -1;
	v4l2_fmtdesc fmt;
	v4l2_frmsizeenum fmtsize;
	
	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_ENUM_FMT, &fmt) >= 0)
	{
		std::string tmp = (char*)fmt.description;
		Logger::LogEx("Camera::ReadFormat index = %d", fmt.index);
		emit OnCameraMessage_Signal("ReadFormat: index = " + QString("%1").arg(fmt.index) + ".");
		Logger::LogEx("Camera::ReadFormat type = %d", fmt.type);
		emit OnCameraMessage_Signal("ReadFormat: type = " + QString("%1").arg(fmt.type) + ".");
		Logger::LogEx("Camera::ReadFormat pixelformat = %d = %s", fmt.pixelformat, V4l2Helper::ConvertPixelformat2String(fmt.pixelformat).c_str());
		emit OnCameraMessage_Signal("ReadFormat: pixelformat = " + QString("%1").arg(fmt.pixelformat) + " = " + QString(V4l2Helper::ConvertPixelformat2String(fmt.pixelformat).c_str()) + ".");
		Logger::LogEx("Camera::ReadFormat description = %s", fmt.description);
		emit OnCameraMessage_Signal("ReadFormat: description = " + QString(tmp.c_str()) + ".");
		
		emit OnCameraPixelformat_Signal(QString("%1").arg(QString(V4l2Helper::ConvertPixelformat2String(fmt.pixelformat).c_str())));
				
		CLEAR(fmtsize);
		fmtsize.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmtsize.pixel_format = fmt.pixelformat;
		fmtsize.index = 0;
		while (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_ENUM_FRAMESIZES, &fmtsize) >= 0)
		{              
			if (fmtsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
			{
				v4l2_frmivalenum fmtival;
				
				Logger::LogEx("Camera::ReadFormat size enum discrete width = %d", fmtsize.discrete.width);
				emit OnCameraMessage_Signal("ReadFormat size enum discrete width = " + QString("%1").arg(fmtsize.discrete.width) + ".");
				Logger::LogEx("Camera::ReadFormat size enum discrete height = %d", fmtsize.discrete.height);
				emit OnCameraMessage_Signal("ReadFormat size enum discrete height = " + QString("%1").arg(fmtsize.discrete.height) + ".");
				
				emit OnCameraFramesize_Signal(QString("disc:%1x%2").arg(fmtsize.discrete.width).arg(fmtsize.discrete.height));
				
				CLEAR(fmtival);
				fmtival.index = 0;
				fmtival.pixel_format = fmt.pixelformat;
				fmtival.width = fmtsize.discrete.width;
				fmtival.height = fmtsize.discrete.height;
				while (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_ENUM_FRAMEINTERVALS, &fmtival) >= 0)
				{
					//Logger::LogEx("Camera::ReadFormat ival type= %d", fmtival.type);
					//emit OnCameraMessage_Signal("ReadFormatsiz ival type = " + QString("%1").arg(fmtival.type) + ".");
					fmtival.index++;
				}
			} 
			else if (fmtsize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
			{
				Logger::LogEx("Camera::ReadFormat size enum stepwise min_width = %d", fmtsize.stepwise.min_width);
				emit OnCameraMessage_Signal("ReadFormat size enum stepwise min_width = " + QString("%1").arg(fmtsize.stepwise.min_width) + ".");
				Logger::LogEx("Camera::ReadFormat size enum stepwise min_height = %d", fmtsize.stepwise.min_height);
				emit OnCameraMessage_Signal("ReadFormat size enum stepwise min_height = " + QString("%1").arg(fmtsize.stepwise.min_height) + ".");
				Logger::LogEx("Camera::ReadFormat size enum stepwise max_width = %d", fmtsize.stepwise.max_width);
				emit OnCameraMessage_Signal("ReadFormat size enum stepwise max_width = " + QString("%1").arg(fmtsize.stepwise.max_width) + ".");
				Logger::LogEx("Camera::ReadFormat size enum stepwise max_height = %d", fmtsize.stepwise.max_height);
				emit OnCameraMessage_Signal("ReadFormat size enum stepwise max_height = " + QString("%1").arg(fmtsize.stepwise.max_height) + ".");
				Logger::LogEx("Camera::ReadFormat size enum stepwise step_width = %d", fmtsize.stepwise.step_width);
				emit OnCameraMessage_Signal("ReadFormat size enum stepwise step_width = " + QString("%1").arg(fmtsize.stepwise.step_width) + ".");
				Logger::LogEx("Camera::ReadFormat size enum stepwise step_height = %d", fmtsize.stepwise.step_height);
				emit OnCameraMessage_Signal("ReadFormat size enum stepwise step_height = " + QString("%1").arg(fmtsize.stepwise.step_height) + ".");
				
				emit OnCameraFramesize_Signal(QString("min:%1x%2,max:%3x%4,step:%5x%6").arg(fmtsize.stepwise.min_width).arg(fmtsize.stepwise.min_height).arg(fmtsize.stepwise.max_width).arg(fmtsize.stepwise.max_height).arg(fmtsize.stepwise.step_width).arg(fmtsize.stepwise.step_height));
			}
			
			result = 0;
			
			fmtsize.index++;
		}
		
		fmt.index++;
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
		fmt.fmt.pix.pixelformat = pixelformat;
		
		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_S_FMT, &fmt))
		{                
			Logger::LogEx("Camera::SetPixelformat to %d OK", pixelformat);
			emit OnCameraMessage_Signal(QString("SetPixelformat: to %1 OK.").arg(pixelformat));
			result = 0;
		}
	}
	else
	{
		Logger::LogEx("Camera::SetPixelformat failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetPixelformat: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
	
    return result;
}

int Camera::ReadPixelformat(uint32_t &pixelformat, QString &pfText)
{
    int result = -1;
	v4l2_format fmt;
	
	CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_FMT, &fmt))
	{                
		pixelformat = fmt.fmt.pix.pixelformat;
		pfText = QString(V4l2Helper::ConvertPixelformat2String(fmt.fmt.pix.pixelformat).c_str());
		
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::ReadPixelformat failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("ReadPixelformat: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
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

		CLEAR(fmt);
		fmt.id = V4L2_CID_GAIN;

		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_CTRL, &fmt))
		{                
			gain = fmt.value;
			
			result = 0;
		}
		else
		{
			Logger::LogEx("Camera::ReadGain failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
			emit OnCameraError_Signal(QString("ReadGain: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		}
	}
	else
	{
		Logger::LogEx("Camera::ReadGain Enum Gain failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
		emit OnCameraMessage_Signal(QString("Enum Gain: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
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
		Logger::LogEx("Camera::SetGain to %d OK", gain);
        emit OnCameraMessage_Signal(QString("SetGain: to %1 OK.").arg(gain));
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::SetGain failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetGain: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
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
		
		CLEAR(fmt);
		fmt.id = V4L2_CID_AUTOGAIN;

		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_CTRL, &fmt))
		{                
			autogain = fmt.value;
			
			result = 0;
		}
		else
		{
			Logger::LogEx("Camera::ReadAutoGain failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
			emit OnCameraError_Signal(QString("ReadAutoGain: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		}
	}
	else
	{
		Logger::LogEx("Camera::ReadAutoGain Enum AutoGain failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
		emit OnCameraMessage_Signal(QString("Enum AutoGain: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
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
		Logger::LogEx("Camera::SetAutoGain to %d OK", ((autogain)?"true":"false"));
        emit OnCameraMessage_Signal(QString("SetAutoGain: to %1 OK.").arg(((autogain)?"true":"false")));
		
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::SetAutoGain failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetAutoGain: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
	
    return result;
}

int Camera::ReadExposure(uint32_t &exposure)
{
    int result = -1;
	v4l2_queryctrl ctrl;
	
	CLEAR(ctrl);
	ctrl.id = V4L2_CID_AUTOGAIN;
	
	if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCTRL, &ctrl) >= 0)
	{
		v4l2_control fmt;
		
		CLEAR(fmt);
		fmt.id = V4L2_CID_EXPOSURE;

		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_CTRL, &fmt))
		{                
			exposure = fmt.value;
			
			result = 0;
		}
		else
		{
			Logger::LogEx("Camera::ReadExposure failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
			emit OnCameraError_Signal(QString("ReadExposure: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		}
	}
	else
	{
		Logger::LogEx("Camera::ReadExposure Enum Exposure failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
		emit OnCameraMessage_Signal(QString("Enum Exposure: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
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
		Logger::LogEx("Camera::SetExposure to %d OK", exposure);
        emit OnCameraMessage_Signal(QString("SetExposure: to %1 OK.").arg(exposure));
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::SetExposure failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetExposure: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
	
    return result;
}

int Camera::ReadAutoExposure(bool &autoexposure)
{
    int result = -1;
	v4l2_queryctrl ctrl;
	
	CLEAR(ctrl);
	ctrl.id = V4L2_CID_AUTOGAIN;
	
	if (V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QUERYCTRL, &ctrl) >= 0)
	{
		v4l2_control fmt;
		
		CLEAR(fmt);
		fmt.id = V4L2_CID_EXPOSURE_AUTO;

		if (-1 != V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_G_CTRL, &fmt))
		{                
			autoexposure = fmt.value;
			
			result = 0;
		}
		else
		{
			Logger::LogEx("Camera::ReadAutoExposure failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
			emit OnCameraError_Signal(QString("ReadAutoExposure: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		}
	}
	else
	{
		Logger::LogEx("Camera::ReadAutoExposure Enum AutoExposure failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
		emit OnCameraMessage_Signal(QString("Enum AutoExposure: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
		
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
		Logger::LogEx("Camera::SetAutoExposure to %d OK", ((autoexposure)?"true":"false"));
        emit OnCameraMessage_Signal(QString("SetAutoExposure: to %1 OK.").arg(((autoexposure)?"true":"false")));
		
		result = 0;
	}
	else
	{
		Logger::LogEx("Camera::SetAutoExposure failed errno=%d=%s", errno, V4l2Helper::ConvertErrno2String(errno).c_str());
        emit OnCameraError_Signal(QString("SetAutoExposure: failed errno=%1=%2.").arg(errno).arg(V4l2Helper::ConvertErrno2String(errno).c_str()));
	}
	
    return result;
}

/*********************************************************************************************************/
// Frame buffer handling
/*********************************************************************************************************/

int Camera::CreateUserBuffer(uint32_t bufferCount, uint32_t bufferSize)
{
    int result = -1;

    if (bufferCount <= MAX_VIEWER_USER_BUFFER_COUNT)
    {
        v4l2_requestbuffers req;

		// creates user defined buffer
		CLEAR(req);

        req.count  = bufferCount;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

		// requests 4 video capture buffer. Driver is going to configure all parameter and doesn't allocate them.
		if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_REQBUFS, &req)) 
		{
			if (EINVAL == errno) 
			{
				Logger::LogEx("Camera::CreateUserBuffer does not support user pointer i/o");
				emit OnCameraError_Signal("Camera::CreateUserBuffer: does not support user pointer i/o.");
            } else {
                Logger::LogEx("Camera::CreateUserBuffer error");
				emit OnCameraError_Signal("Camera::CreateUserBuffer: error.");
            }
        }

        // create local buffer container
        m_UserBufferContainerList.resize(bufferCount);
        
        if (m_UserBufferContainerList.size() != bufferCount) 
		{
			Logger::LogEx("Camera::CreateUserBuffer buffer container error");
			emit OnCameraError_Signal("Camera::CreateUserBuffer: buffer container error.");
			return -1;
        }

        // get the length and start address of each of the 4 buffer structs and assign the user buffer addresses
        for (int x = 0; x < bufferCount; ++x) 
		{
			m_UserBufferContainerList[x] = new USER_BUFFER;
            m_UserBufferContainerList[x]->nBufferlength = bufferSize;
            m_UserBufferContainerList[x]->pBuffer = new uint8_t[bufferSize];

            if (!m_UserBufferContainerList[x]->pBuffer) 
			{
				Logger::LogEx("Camera::CreateUserBuffer buffer creation error");
				emit OnCameraError_Signal("Camera::CreateUserBuffer: buffer creation error.");
				return -1;
            }
        }

        m_UsedBufferCount = bufferCount;
        result = 0;
    }

    return result;
}

int Camera::QueueAllUserBuffer()
{
	int result = -1;
	
    // queue the buffer
    for (uint32_t i=0; i<m_UsedBufferCount; i++)
    {
		v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;
		buf.index = i;
		buf.m.userptr = (unsigned long)m_UserBufferContainerList[i]->pBuffer;
		buf.length = m_UserBufferContainerList[i]->nBufferlength;

		if (-1 == V4l2Helper::xioctl(m_nFileDescriptor, VIDIOC_QBUF, &buf))
		{
			Logger::LogEx("Camera::QueueUserBuffer queue #%d buffer=%p failed", i, m_UserBufferContainerList[i]->pBuffer);
			return result;
		}
        else
		{
            Logger::LogEx("Camera::QueueUserBuffer queue #%d buffer=%p OK", i, m_UserBufferContainerList[i]->pBuffer);
			result = 0;
		}
    }
    
    return result;
}

int Camera::QueueSingleUserBuffer(const uint64_t frameHandle)
{
	int result = 0;
    
	/*
	int nResult = libcsi_queue_frame(m_DeviceHandle, (frame_handle_t*)frameHandle, FrameCallback, &m_DmaSICallbacks);

    if (nResult != 0)
        Logger::LogEx("Camera::QueueSingleUserBuffer libcsi_queue_frame devicehandle=%p framehandle=%p failed, result=%d=%s", m_DeviceHandle, frameHandle, nResult, ConvertResult2String(nResult).c_str());
    else
        Logger::LogEx("Camera::QueueSingleUserBuffer libcsi_queue_frame devicehandle=%p framehandle=%p OK", m_DeviceHandle, frameHandle);
        */
	
	return result;
}

int Camera::DeleteUserBuffer()
{
    int result = 0;

    // delete all user buffer
	for (int x = 0; x < m_UsedBufferCount; x++)
	{
		delete [] m_UserBufferContainerList[x]->pBuffer;
		
		delete m_UserBufferContainerList[x];
	}
	
	m_UserBufferContainerList.resize(0);
	
    return result;
}

/*********************************************************************************************************/
// Info
/*********************************************************************************************************/

uint32_t GetUINT32(std::string tmp)
{
    uint32_t result = 0;

    if (tmp.find("0x") != std::string::npos)
    {
        tmp = tmp.substr(2, tmp.size() - tmp.find("0x") - 2);
        result = strtoul((const char*)tmp.c_str(), 0, 16); 
    }
    else
    {
        result = strtoul((const char*)tmp.c_str(), 0, 10); 
    }

    return result;
}

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
        Logger::LogEx("Camera::GetCameraDriverName %s is no V4L2 device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraDriverName " + QString(m_DeviceName.c_str()) + " is no V4L2 device.");
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
        Logger::LogEx("Camera::GetCameraDriverName %s is no video capture device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraDriverName " + QString(m_DeviceName.c_str()) + " is no video capture device.");
        return -1;
    }
        
    strText = (char*)cap.driver;
    //fprintf(stderr, "cap.driver       = %s\n", cap.driver);
	//fprintf(stderr, "cap.card         = %s\n", cap.card);
	//fprintf(stderr, "cap.bus_info     = %s\n", cap.bus_info);
	//fprintf(stderr, "cap.version      = 0x%08x\n", cap.version);
	//fprintf(stderr, "cap.capabilities = 0x%08x\n", cap.capabilities);

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

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
        Logger::LogEx("Camera::GetCameraDeviceName %s is no video capture device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraDeviceName " + QString(m_DeviceName.c_str()) + " is no video capture device.");
        return -1;
    }
        
    strText = (char*)cap.card;
    //fprintf(stderr, "cap.driver       = %s\n", cap.driver);
	//fprintf(stderr, "cap.card         = %s\n", cap.card);
	//fprintf(stderr, "cap.bus_info     = %s\n", cap.bus_info);
	//fprintf(stderr, "cap.version      = 0x%08x\n", cap.version);
	//fprintf(stderr, "cap.capabilities = 0x%08x\n", cap.capabilities);

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

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
        Logger::LogEx("Camera::GetCameraBusInfo %s is no video capture device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraBusInfo " + QString(m_DeviceName.c_str()) + " is no video capture device.");
        return -1;
    }
        
    strText = (char*)cap.bus_info;
    //fprintf(stderr, "cap.driver       = %s\n", cap.driver);
	//fprintf(stderr, "cap.card         = %s\n", cap.card);
	//fprintf(stderr, "cap.bus_info     = %s\n", cap.bus_info);
	//fprintf(stderr, "cap.version      = 0x%08x\n", cap.version);
	//fprintf(stderr, "cap.capabilities = 0x%08x\n", cap.capabilities);

    return result;
}

int Camera::GetCameraDriverVersion(uint32_t index, std::string &strText)
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
        Logger::LogEx("Camera::GetCameraDriverVersion %s is no V4L2 device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraDriverVersion " + QString(m_DeviceName.c_str()) + " is no V4L2 device.");
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
        Logger::LogEx("Camera::GetCameraDriverVersion %s is no video capture device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraDriverVersion " + QString(m_DeviceName.c_str()) + " is no video capture device.");
        return -1;
    }
        
    std::stringstream tmp;
	tmp << ((cap.version/0x10000)&0xFF) << "." << ((cap.version/0x100)&0xFF) << "." << (cap.version&0xFF);
    strText = tmp.str();
    //fprintf(stderr, "cap.driver       = %s\n", cap.driver);
	//fprintf(stderr, "cap.card         = %s\n", cap.card);
	//fprintf(stderr, "cap.bus_info     = %s\n", cap.bus_info);
	//fprintf(stderr, "cap.version      = 0x%08x\n", cap.version);
	//fprintf(stderr, "cap.capabilities = 0x%08x\n", cap.capabilities);

    return result;
}

int Camera::GetCameraCapabilities(uint32_t index, std::string &strText)
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
        Logger::LogEx("Camera::GetCameraCapabilities %s is no V4L2 device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraCapabilities " + QString(m_DeviceName.c_str()) + " is no V4L2 device.");
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
        Logger::LogEx("Camera::GetCameraCapabilities %s is no video capture device\n", m_DeviceName.c_str());
		emit OnCameraError_Signal("Camera::GetCameraCapabilities " + QString(m_DeviceName.c_str()) + " is no video capture device.");
        return -1;
    }
        
    std::stringstream tmp;
	tmp << "0x" << std::hex << cap.capabilities << std::endl
		<< "    Read/Write = " << ((cap.capabilities & V4L2_CAP_READWRITE)?"Yes":"No") << std::endl
		<< "    Streaming = " << ((cap.capabilities & V4L2_CAP_STREAMING)?"Yes":"No");
    strText = tmp.str();
    //fprintf(stderr, "cap.driver       = %s\n", cap.driver);
	//fprintf(stderr, "cap.card         = %s\n", cap.card);
	//fprintf(stderr, "cap.bus_info     = %s\n", cap.bus_info);
	//fprintf(stderr, "cap.version      = 0x%08x\n", cap.version);
	//fprintf(stderr, "cap.capabilities = 0x%08x\n", cap.capabilities);

    return result;
}

    
// Recording
void Camera::SetRecording(bool start)
{
    m_DmaSICallbacks.SetRecording(start);
}

void Camera::DisplayStepBack()
{
    m_DmaSICallbacks.DisplayStepBack();
}

void Camera::DisplayStepForw()
{
    m_DmaSICallbacks.DisplayStepForw();
}

void Camera::DeleteRecording()
{
    m_DmaSICallbacks.DeleteRecording();
}

/*********************************************************************************************************/
// Commands
/*********************************************************************************************************/

/*********************************************************************************************************/
// Tools
/*********************************************************************************************************/



}}} // namespace AVT::Tools::Examples

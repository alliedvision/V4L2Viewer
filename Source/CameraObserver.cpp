#include "V4l2Helper.h"
#include "Logger.h"
#include "CameraObserver.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

namespace AVT {
namespace Tools {
namespace Examples {

////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////    

CameraObserver::CameraObserver(void)
    : m_bTerminate(false)
	, m_bAbort(false)
{
    
}


CameraObserver::~CameraObserver(void)
{
	Stop();
}

void CameraObserver::Start()
{
	start();
}

void CameraObserver::Stop()
{
	// stop the internal processing thread and wait until the thread is really stopped
	m_bAbort = true;

	// wait until the thread is stopped
	while (isRunning())
		QThread::msleep(10);
}

void CameraObserver::SetTerminateFlag()
{
    m_bTerminate = true;
}

// Do the work within this thread
void CameraObserver::run()
{
	while (!m_bAbort)
	{
		CheckDevices();
		
		QThread::msleep(1000);
	}
}

int CameraObserver::CheckDevices()
{
	uint32_t device_count = 0;
    int nResult = 0;
	QString deviceName;
	int fileDiscriptor = -1;
	std::map<int, std::string>	deviceList;
	
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
				deviceList[device_count] = deviceName.toStdString();
					
				device_count++;
				
				Logger::LogEx("Camera::DeviceDiscoveryStart close %s OK", deviceName.toAscii().data());
				emit OnCameraMessage_Signal("DeviceDiscoveryStart: close " + deviceName + " OK");
			}
		}
	}
	while(++device_count < 10);
	
	for (int i=0; i<deviceList.size(); i++)
	{
		bool found = false;
		
		for (int ii=0; ii<m_DeviceList.size(); ii++)
		{
			if (deviceList[i] == m_DeviceList[ii])
			{
				found = true;
				break;
			}
		}
		
		if (!found)
		{
			int ii = 0;
			for (ii=0; ii<m_DeviceList.size(); ii++)
			{
				bool found = false;
				for (std::map<int, std::string>::iterator iii=m_DeviceList.begin(); iii!=m_DeviceList.end(); iii++)
				{
					if (iii->first == ii)
					{
						found = true;
						break;
					}
				}
				if (found == false)
					break;
			}
			m_DeviceList[ii] = deviceList[i].c_str();
			emit OnCameraListChanged_Signal(UpdateTriggerPluggedIn, 0, ii, m_DeviceList[ii].c_str(), "");
		}
	}
	
    for (int i=0; i<m_DeviceList.size(); i++)
	{
		bool found = false;
		
		for (int ii=0; ii<deviceList.size(); ii++)
		{
			if (deviceList[ii] == m_DeviceList[i])
			{
				found = true;
				break;
			}
		}
		
		if (!found)
		{
			m_DeviceList.erase(i);
			emit OnCameraListChanged_Signal(UpdateTriggerPluggedOut, 0, i, m_DeviceList[i].c_str(), "");
		}
	}
	
    return 0;
}

// Callbacks

void CameraObserver::OnDeviceReady(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData)
{
    if (!m_bTerminate)
    {
        emit OnCameraListChanged_Signal(UpdateTriggerPluggedIn, cardNumber, deviceID, "", "");
    }
    
}

void CameraObserver::OnDeviceRemoved(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData)
{
    if (!m_bTerminate)
    {
        emit OnCameraListChanged_Signal(UpdateTriggerPluggedOut, cardNumber, deviceID, "", "");
    }
}
	
void CameraObserver::OnMessage(const char *text, void *pPrivateData) 
{
	if (!m_bTerminate)
    {
        Logger::LogEx("CameraObserver: message = '%s'", text);
    }
}

}}} // namespace AVT::Tools::Examples
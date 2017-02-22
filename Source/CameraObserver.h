#ifndef CAMERAOBSERVER_INCLUDE
#define CAMERAOBSERVER_INCLUDE

#include <map>
#include <QObject>
#include <QThread>
#include <QImage>
#include <QSharedPointer>

#include <stdint.h>

enum UpdateTriggerType
{
    UpdateTriggerPluggedIn           = 0,
    UpdateTriggerPluggedOut          = 1,
    UpdateTriggerOpenStateChanged    = 3
};

namespace AVT {
namespace Tools {
namespace Examples {
	
class CameraObserver : public QThread
{
    Q_OBJECT

public:
	CameraObserver(void);
	virtual ~CameraObserver(void);

    void Start();
    void Stop();

    void SetTerminateFlag();

    // Implementation
	// Do the work within this thread
	virtual void run();
    
	int CheckDevices();
	
    // Callbacks

    // Device
    virtual void OnDeviceReady(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData);
    virtual void OnDeviceRemoved(uint32_t cardNumber, uint64_t deviceID, const void *pPrivateData);
	
	
private:
	// Callbacks

    // Messages
	virtual void OnMessage(const char *text, void *pPrivateData);
	
signals:
    // The camera list changed signal that passes the new camera and the its state directly
    void OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &);
	// Event will be called on error
	void OnCameraError_Signal(const QString &text);
    // Event will be called on message
	void OnCameraMessage_Signal(const QString &text);
    
private:
    bool                                        m_bTerminate;

	// Variable to abort the running thread
	bool 										m_bAbort;
	
	std::map<int, std::string>					m_DeviceList;
	
};

}}} // namespace AVT::Tools::Examples

#endif /* CAMERAOBSERVER_INCLUDE */

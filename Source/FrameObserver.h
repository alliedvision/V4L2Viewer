/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameObserver.h

  Description: The frame observer that is used for notifications
               regarding the arrival of a newly acquired frame.

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

#ifndef FRAMEOBSERVER_INCLUDE
#define FRAMEOBSERVER_INCLUDE

#include <queue>

#include <QObject>
#include <QThread>
#include <QImage>
#include <QSharedPointer>

#include "MyFrameQueue.h"
#include "ImageProcessingThread.h"
#include "Helper.h"
#include "V4l2Helper.h"

////////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////////

#define MAX_VIEWER_USER_BUFFER_COUNT	50

////////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////////


namespace AVT {
namespace Tools {
namespace Examples {

typedef struct _USER_BUFFER
{
    uint8_t         *pBuffer;
    size_t  		nBufferlength;
} USER_BUFFER, *PUSER_BUFFER, **PPUSER_BUFFER;


class FrameObserver : public QThread
{
    Q_OBJECT

  public:
    // We pass the camera that will deliver the frames to the constructor
	FrameObserver();
    //
	virtual ~FrameObserver();

	int StartStream(bool blockingMode, int fileDescriptor, uint32_t pixelformat, uint32_t payloadsize, uint32_t width, uint32_t height, uint32_t bytesPerLine);
	int StopStream();
	
    // Get the number of frames
	// This function will clear the counter of received frames
	unsigned int GetReceivedFramesCount();

	// Get the number of uncompleted frames
	unsigned int GetDroppedFramesCount();

	// Set the number of uncompleted frames
	void ResetDroppedFramesCount();

    // switch to stop callbacks immediately
    void SetTerminateFlag();

    // Recording
    void SetRecording(bool start);
    void DisplayStepBack();
    void DisplayStepForw();
    void DeleteRecording();
    
    virtual int CreateUserBuffer(uint32_t bufferCount, uint32_t bufferSize);
    virtual int QueueAllUserBuffer();
    virtual int QueueSingleUserBuffer(const int index);
    virtual int DeleteUserBuffer();
    
    void FrameDone(const unsigned long long frameHandle);

protected:
	// v4l2
    int DisplayFrame(const uint8_t* pBuffer, uint32_t length,
                     QImage &convertedImage);
    virtual int ReadFrame();
	
	// Do the work within this thread
    virtual void run();
	
protected:
    const static int MAX_RECORD_FRAME_QUEUE_SIZE = 100;
    bool m_bRecording;
    MyFrameQueue m_FrameRecordQueue;

	// Counter to count the received images
	uint32_t m_nReceivedFramesCounter;

	// Counter to count the received uncompleted images
	unsigned int m_nDroppedFramesCounter;

    // switch to stop callbacks immediately
    bool m_bTerminate;

    // Variable to abort the running thread
	bool m_bAbort;
	int m_nFileDescriptor;
	uint32_t m_Pixelformat;
	uint32_t m_nWidth;
	uint32_t m_nHeight;
	uint32_t m_PayloadSize;
	uint32_t m_BytesPerLine;
	uint64_t m_FrameId;
	
	bool m_MessageSendFlag;
        bool m_BlockingMode;
	bool m_bStreamRunning;
	bool m_bStreamStopped;
	
      	std::vector<PUSER_BUFFER>					m_UserBufferContainerList;

	uint32_t                                    m_UsedBufferCount;
	
	// Shared pointer to a worker thread for the image processing
	QSharedPointer<ImageProcessingThread> m_pImageProcessingThread;

private slots:
	//Event handler for getting the processed frame to an image
	void OnFrameReadyFromThread(const QImage &image, const unsigned long long &frameId);
	
signals:
	// Event will be called when a frame is processed by the internal thread and ready to show
	void OnFrameReady_Signal(const QImage &image, const unsigned long long &frameId);
	// Event will be called when the frame processing is done and the frame can be returned to streaming engine
    void OnFrameDone_Signal(const unsigned long long frameHandle);
    // Event will be called when the a frame is recorded
	void OnRecordFrame_Signal(const unsigned long long &, const unsigned long long &);
    // Event will be called when the a frame is displayed
	void OnDisplayFrame_Signal(const unsigned long long &);
    // Event will be called when for text notification
    void OnMessage_Signal(const QString &msg);
    // Event will be called on error
    void OnError_Signal(const QString &text);
};

}}} // namespace AVT::Tools::Examples

#endif /* FRAMEOBSERVER_INCLUDE */


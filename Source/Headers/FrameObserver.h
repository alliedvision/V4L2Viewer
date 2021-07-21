/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

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

#ifndef FRAMEOBSERVER_H
#define FRAMEOBSERVER_H

#include "ImageProcessingThread.h"
#include "LocalMutex.h"
#include <QImage>
#include <QObject>
#include <QSharedPointer>
#include <QThread>

#include <fcntl.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <queue>
#include <vector>
#include "V4L2Helper.h"

#define MAX_VIEWER_USER_BUFFER_COUNT    50

struct UserBuffer
{
    uint8_t         *pBuffer;
    size_t          nBufferlength;
};


class FrameObserver : public QThread
{
    Q_OBJECT

public:
    // We pass the camera that will deliver the frames to the constructor
    FrameObserver(bool showFrames);

    virtual ~FrameObserver();

    // This function starts streaming
    //
    // Parameters:
    // [in] (bool) blockingMode
    // [in] (int) fileDescriptor
    // [in] (uint32_t) pixelFormat
    // [in] (uint32_t) payloadSize
    // [in] (uint32_t) width - width of the frame
    // [in] (uint32_t) height - height of the frame
    // [in] (uint32_t) bytesPerLine
    // [in] (uint32_t) enableLogging
    //
    // Returns:
    // (int) - result of stream starting
    int StartStream(bool blockingMode, int fileDescriptor, uint32_t pixelFormat,
                    uint32_t payloadSize, uint32_t width, uint32_t height, uint32_t bytesPerLine,
                    uint32_t enableLogging);
    // This function stops streaming
    //
    // Returns:
    // (int) - result of stream stopping
    int StopStream();

    // Get the number of frames
    // This function will return counter of the received frames
    //
    // Returns:
    // (unsigned int) - received frames count
    unsigned int GetReceivedFramesCount();
    // This function will return counter of the rendered frames
    //
    // Returns:
    // (unsigned int) - rendered frames count
    unsigned int GetRenderedFramesCount();

    // This function will return counter of the dropped frames
    //
    // Returns:
    // (unsigned int) - dropped frames count
    unsigned int GetDroppedFramesCount();

    // This function sets the number of uncompleted frames
    void ResetDroppedFramesCount();

    // This function sets file descriptor
    //
    // Parameters:
    // [in] (int) fd - file descriptor
    void setFileDescriptor(int fd);

    // This function creates all user buffer
    //
    // Parameters:
    // [in] (uint32_t) bufferCount
    // [in] (uint32_t) bufferSize
    //
    // Returns:
    // (int) - result of the buffer creation
    virtual int CreateAllUserBuffer(uint32_t bufferCount, uint32_t bufferSize);
    // This function queues all user buffer
    //
    // Returns:
    // (int) - result of the buffer queuing
    virtual int QueueAllUserBuffer();
    // This function queues single user buffer
    //
    // Parameters:
    // [in] (const int) index - index of the buffer
    //
    // Returns:
    // (int) - result of the buffer queuing
    virtual int QueueSingleUserBuffer(const int index);
    // This function removes all user buffer
    //
    // Returns:
    // (int) - result of the buffer removal
    virtual int DeleteAllUserBuffer();

    // This function switches on/off frame transfer to gui
    //
    // Parameters:
    // [in] (bool) showFrames
    void SwitchFrameTransfer2GUI(bool showFrames);

protected:
    // v4l2
    // This function reads frame
    //
    // Parameters:
    // [in] (v4l2_buffer &) buf - buffer of the frame
    //
    // Returns:
    // (int) - result of frame reading
    virtual int ReadFrame(v4l2_buffer &buf);
    // This function returns frame data
    //
    // Parameters:
    // [in] (v4l2_buffer &) buf
    // [in] (uint8_t *&) buffer
    // [in] (uint32_t &) length - length of the buffer
    //
    // Returns:
    // (int) - result of getting data
    virtual int GetFrameData(v4l2_buffer &buf, uint8_t *&buffer, uint32_t &length);
    // This function process frame from the buffer given in parameter
    //
    // Parameters:
    // [in] (v4l2_buffer &) buf - given buffer
    //
    // Returns:
    // (int) - result of frame processing
    int ProcessFrame(v4l2_buffer &buf);
    // This function dequeues and process current frame
    void DequeueAndProcessFrame();

    // This function does the work within this thread
    virtual void run();

protected:
    // Counter to count the received images
    uint32_t m_nReceivedFramesCounter;

    // Counter to count the rendered frames
    uint32_t m_nRenderedFramesCounter;

    // Counter to count the received uncompleted images
    unsigned int m_nDroppedFramesCounter;

    int m_nFileDescriptor;
    uint32_t m_PixelFormat;
    uint32_t m_nWidth;
    uint32_t m_nHeight;
    uint32_t m_PayloadSize;
    uint32_t m_RealPayloadSize;
    uint32_t m_BytesPerLine;
    uint64_t m_FrameId;
    uint32_t m_DQBUF_last_errno;

    bool m_MessageSendFlag;
    bool m_BlockingMode;
    bool m_IsStreamRunning;
    bool m_bStreamStopped;

    uint32_t m_EnableLogging;

    bool m_ShowFrames;

    std::vector<UserBuffer*>              m_UserBufferContainerList;
    base::LocalMutex                      m_UsedBufferMutex;

    // Shared pointer to a worker thread for the image processing
    QSharedPointer<ImageProcessingThread> m_pImageProcessingThread;

private slots:
    //Event handler for getting the processed frame to an image
    void OnFrameReadyFromThread(const QImage &image, const unsigned long long &frameId, const int &bufIndex);

signals:
    // Event will be called when a frame is processed by the internal thread and ready to show
    void OnFrameReady_Signal(const QImage &image, const unsigned long long &frameId);
    // Event will be called when a frame is processed by the internal thread and ready to show
    void OnFrameID_Signal(const unsigned long long &frameId);
    // Event will be called when the frame processing is done and the frame can be returned to streaming engine
    //void OnFrameDone_Signal(const unsigned long long frameHandle);
    // Event will be called when the a frame is displayed
    void OnDisplayFrame_Signal(const unsigned long long &);
};

#endif /* FRAMEOBSERVER_H */


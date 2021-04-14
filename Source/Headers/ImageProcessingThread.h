/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ImageProcessingThread.h

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

#ifndef IMAGEPORCESSINGTHREAD_H
#define IMAGEPORCESSINGTHREAD_H

#include <MyFrame.h>
#include <MyFrameQueue.h>

#include <QImage>
#include <QObject>
#include <QSharedPointer>
#include <QThread>

class ImageProcessingThread : public QThread
{
    Q_OBJECT

public:
    ImageProcessingThread();
    ~ImageProcessingThread(void);

    // Queue the frame for the thread to work with
    int QueueFrame(uint32_t &bufferIndex, uint8_t *&buffer, uint32_t &length,
                   uint32_t &width, uint32_t &height, uint32_t &pixelFormat,
                   uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID);

    // Queue the frame for the thread to work with
    int QueueFrame(QImage &image, uint64_t &frameID);

    // Queue the frame for the thread to work with
    int QueueFrame(QSharedPointer<MyFrame> pFrame);

    // start thread
    void StartThread();

    // stop the internal processing thread and wait until the thread is really stopped
    void StopThread();

protected:
    // Do the work within this thread
    virtual void run();

private:
    const static int MAX_QUEUE_SIZE = 1;

    // Frame queue
    MyFrameQueue m_FrameQueue;

    // Variable to abort the running thread
    bool m_bAbort;

signals:
    // Event will be called when an image is processed by the thread
    void OnFrameReady_Signal(const QImage &image, const unsigned long long &frameId, const int &bufIndex);

    // Event will be called when message should be displayed
    void OnMessage_Signal(const QString &msg);
};

#endif // IMAGEPORCESSINGTHREAD_H


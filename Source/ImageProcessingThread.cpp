/*=============================================================================
  Copyright (C) 2015 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ImageProcessingThread.cpp

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

#include "ImageProcessingThread.h"


ImageProcessingThread::ImageProcessingThread()
	: m_bAbort(false)
{
}

ImageProcessingThread::~ImageProcessingThread(void)
{
}

// Set the data for the thread to work with
void ImageProcessingThread::QueueFrame(QImage &image, uint64_t &frameID)
{
    // make sure Viewer is never working in the past
    // if viewer is too slow we drop the frames.
    if (m_FrameQueue.GetSize() < MAX_QUEUE_SIZE)
	    m_FrameQueue.Enqueue(image, frameID);
}

// Set the data for the thread to work with
void ImageProcessingThread::QueueFrame(QSharedPointer<MyFrame> pFrame)
{
    // make sure Viewer is never working in the past
    // if viewer is too slow we drop the frames.
    if (m_FrameQueue.GetSize() < MAX_QUEUE_SIZE)
	    m_FrameQueue.Enqueue(pFrame);
}

// stop the internal processing thread and wait until the thread is really stopped
void ImageProcessingThread::StopThread()
{
	m_bAbort = true;

	// wait until the thread is stopped
	while (isRunning())
		QThread::msleep(10);
}

// Do the work within this thread
void ImageProcessingThread::run()
{
	while (!m_bAbort)
	{
		if(0 < m_FrameQueue.GetSize())
		{
			QSharedPointer<MyFrame> pFrame = m_FrameQueue.Dequeue();
			uint64_t frameID = pFrame->GetFrameId();
			QImage image = pFrame->GetImage();

			emit OnFrameReady_Signal(image, frameID);
		}

		QThread::msleep(10);
	}
}


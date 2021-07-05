/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameObserverRead.h

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

#ifndef FRAMEOBSERVERREAD_H
#define FRAMEOBSERVERREAD_H

#include "FrameObserver.h"

class FrameObserverRead : public FrameObserver
{
  public:
    // We pass the camera that will deliver the frames to the constructor
    FrameObserverRead(bool showFrames);

    virtual ~FrameObserverRead();

    virtual int CreateAllUserBuffer(uint32_t bufferCount, uint32_t bufferSize);
    virtual int QueueAllUserBuffer();
    virtual int QueueSingleUserBuffer(const int index);
    virtual int DeleteAllUserBuffer();

protected:
    // v4l2
    virtual int ReadFrame(v4l2_buffer &buf);
    virtual int GetFrameData(v4l2_buffer &buf, uint8_t *&buffer, uint32_t &length);

private:
    int        m_nFrameBufferIndex;
};

#endif // FRAMEOBSERVERREAD_H

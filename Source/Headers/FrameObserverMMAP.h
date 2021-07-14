/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameObserverMMAP.h

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

#ifndef FRAMEOBSERVERMMAP_H
#define FRAMEOBSERVERMMAP_H

#include "FrameObserver.h"

class FrameObserverMMAP : public FrameObserver
{
  public:
    // We pass the camera that will deliver the frames to the constructor
    FrameObserverMMAP(bool showFrames);

    virtual ~FrameObserverMMAP();

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

private:

};

#endif // FRAMEOBSERVERMMAP_H


/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2021 Allied Vision Technologies GmbH

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.  */


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


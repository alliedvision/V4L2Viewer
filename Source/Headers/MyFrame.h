/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        MyFrame.h

  Description: This class contains information and all functions about
               camera's frame.

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

#ifndef MYFRAME_H
#define MYFRAME_H

#include <QImage>

#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <vector>

class MyFrame
{
public:
    // Copy the of the given frame
    MyFrame(const MyFrame *pFrame);

    MyFrame(QImage &image, unsigned long long frameID);

    // constructor used by image processing thread
    MyFrame(uint32_t &bufferIndex, uint8_t *&buffer, uint32_t &length,
            uint32_t &width, uint32_t &height, uint32_t &pixelFormat,
            uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID);

    // constructor used by recorder
    MyFrame(QImage &image, uint32_t &bufferIndex, uint8_t *&buffer, uint32_t &length,
            uint32_t &width, uint32_t &height, uint32_t &pixelFormat,
            uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID);

    ~MyFrame(void);

    // This function returns the frame buffer
    //
    // Returns:
    // (QImage &) - reference to the frame
    QImage &GetImage();

    // This function returns the id of the frame
    //
    // Returns:
    // (unsigned long long) - frame id
    unsigned long long   GetFrameId();
    // This function returns buffer
    //
    // Returns:
    // (uint8_t *) - buffer
    uint8_t *            GetBuffer();
    // This function returns buffer length
    //
    // Returns:
    // (uint32_t) - length
    uint32_t             GetBufferlength();
    // This function returns frame width
    //
    // Returns:
    // (uint32_t) - width
    uint32_t             GetWidth();
    // This function returns frame height
    //
    // Returns:
    // (uint32_t) - height
    uint32_t             GetHeight();
    // This function returns pixel format
    //
    // Returns:
    // (uint32_t) - pixel format
    uint32_t             GetPixelFormat();
    // This function returns payload size
    //
    // Returns:
    // (uint32_t) - payload size
    uint32_t             GetPayloadSize();
    // This function returns bytes per line
    //
    // Returns:
    // (uint32_t) - bytes per line
    uint32_t             GetBytesPerLine();
    // This function returns buffer index
    //
    // Returns:
    // (uint32_t) - buffer index
    uint32_t             GetBufferIndex();

private:
    QImage               m_Image;
    std::vector<uint8_t> m_Buffer;
    uint8_t *            m_pBuffer;
    uint64_t             m_FrameId;
    uint32_t             m_Width;
    uint32_t             m_Height;
    uint32_t             m_PixelFormat;
    uint32_t             m_PayloadSize;
    uint32_t             m_BytesPerLine;
    uint32_t             m_Bufferlength;
    uint32_t             m_BufferIndex;
};

#endif // MYFRAME_H


/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        MyFrame.h

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

    // Get the frame buffer
    QImage &GetImage();

    // get the id of the frame
    unsigned long long   GetFrameId();
    uint8_t *            GetBuffer();
    uint32_t             GetBufferlength();
    uint32_t             GetWidth();
    uint32_t             GetHeight();
    uint32_t             GetPixelFormat();
    uint32_t             GetPayloadSize();
    uint32_t             GetBytesPerLine();
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


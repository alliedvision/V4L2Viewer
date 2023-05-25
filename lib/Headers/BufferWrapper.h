#ifndef BUFFERWRAPPER_H
#define BUFFERWRAPPER_H

#include <cstdint>
#include <cstdlib>
#include <linux/videodev2.h>

struct BufferWrapper
{
    v4l2_buffer buffer;
    uint8_t const* data;
    size_t length;
    uint32_t width;
    uint32_t height;
    uint32_t pixelFormat;
    uint32_t payloadSize;
    uint32_t bytesPerLine;
    uint64_t frameID;
};

#endif

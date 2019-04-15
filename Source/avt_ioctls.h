#ifndef AVT_IOCTLS
#define AVT_IOCTLS

/* i2c read */
#define VIDIOC_R_I2C                        _IOWR('V', BASE_VIDIOC_PRIVATE + 0,  struct v4l2_i2c)

/* i2c write */
#define VIDIOC_W_I2C                        _IOWR('V', BASE_VIDIOC_PRIVATE + 1,  struct v4l2_i2c)

/* Stream statistics */
#define VIDIOC_STREAM_STAT                  _IOWR('V', BASE_VIDIOC_PRIVATE + 5, struct v4l2_stats_t)


struct v4l2_i2c
{
    __u32       nRegisterAddress;       // Register
    __u32       nTimeout;               // Timeout value
    const char  *pBuffer;               // I/O buffer
    __u32       nRegisterSize;          // Register size
    __u32       nNumBytes;              // Bytes to read
};

struct v4l2_stats_t
{
    __u64    FramesCount;           // Total number of frames received
    __u64    PacketCRCError;        // Number of packets with CRC errors
    __u64    FramesUnderrun;        // Number of frames dropped because of buffer underrun
    __u64    FramesIncomplete;      // Number of frames that were not completed
    __u64    CurrentFrameCount;     // Number of frames received within CurrentFrameInterval (nec. to calculate fps value)
    __u64    CurrentFrameInterval;  // Time interval between frames in µs
};


#endif // AVT_IOCTLS
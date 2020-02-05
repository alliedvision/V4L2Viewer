#ifndef VIDEODEV2_AV_H
#define VIDEODEV2_AV_H

#include <linux/videodev2.h>

/* 10bit and 12bit greyscale packed */
#define V4L2_PIX_FMT_Y10P     v4l2_fourcc('Y', '1', '0', 'P')
#define V4L2_PIX_FMT_Y12P     v4l2_fourcc('Y', '1', '2', 'P')

/* 10bit raw bayer packed, 5 bytes for every 4 pixels */
#define V4L2_PIX_FMT_SBGGR10P v4l2_fourcc('p', 'B', 'A', 'A')
#define V4L2_PIX_FMT_SGBRG10P v4l2_fourcc('p', 'G', 'A', 'A')
#define V4L2_PIX_FMT_SGRBG10P v4l2_fourcc('p', 'g', 'A', 'A')
#define V4L2_PIX_FMT_SRGGB10P v4l2_fourcc('p', 'R', 'A', 'A')

/* 12bit raw bayer packed, 6 bytes for every 4 pixels */
#define V4L2_PIX_FMT_SBGGR12P v4l2_fourcc('p', 'B', 'C', 'C')
#define V4L2_PIX_FMT_SGBRG12P v4l2_fourcc('p', 'G', 'C', 'C')
#define V4L2_PIX_FMT_SGRBG12P v4l2_fourcc('p', 'g', 'C', 'C')
#define V4L2_PIX_FMT_SRGGB12P v4l2_fourcc('p', 'R', 'C', 'C')

#ifndef V4L2_PIX_FMT_XBGR32
#define V4L2_PIX_FMT_XBGR32 v4l2_fourcc('X', 'R', '2', '4')
#endif

#ifndef V4L2_PIX_FMT_ABGR32
#define V4L2_PIX_FMT_ABGR32 v4l2_fourcc('A', 'R', '2', '4')
#endif

#ifndef V4L2_PIX_FMT_XRGB32
#define V4L2_PIX_FMT_XRGB32 v4l2_fourcc('B', 'X', '2', '4')
#endif

#endif // VIDEODEV2_AV_H

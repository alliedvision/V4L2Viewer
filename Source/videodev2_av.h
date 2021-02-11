/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        videodev2_av.h

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

#ifndef VIDEODEV2_AV_H
#define VIDEODEV2_AV_H

#include <linux/videodev2.h>

/* 10bit and 12bit greyscale packed */
#define V4L2_PIX_FMT_Y10P     v4l2_fourcc('Y', '1', '0', 'P')
#define V4L2_PIX_FMT_Y12P     v4l2_fourcc('Y', '1', '2', 'P')

/* Jetson TX2 pixel formats */
#define V4L2_PIX_FMT_TX2_Y10     v4l2_fourcc('J', '2', 'Y', '0')
#define V4L2_PIX_FMT_TX2_Y12     v4l2_fourcc('J', '2', 'Y', '2')
#define V4L2_PIX_FMT_TX2_SRGGB10 v4l2_fourcc('J', '2', 'R', '0')
#define V4L2_PIX_FMT_TX2_SGRBG10 v4l2_fourcc('J', '2', 'A', '0')
#define V4L2_PIX_FMT_TX2_SGBRG10 v4l2_fourcc('J', '2', 'G', '0')
#define V4L2_PIX_FMT_TX2_SBGGR10 v4l2_fourcc('J', '2', 'B', '0')
#define V4L2_PIX_FMT_TX2_SRGGB12 v4l2_fourcc('J', '2', 'R', '2')
#define V4L2_PIX_FMT_TX2_SGRBG12 v4l2_fourcc('J', '2', 'A', '2')
#define V4L2_PIX_FMT_TX2_SGBRG12 v4l2_fourcc('J', '2', 'G', '2')
#define V4L2_PIX_FMT_TX2_SBGGR12 v4l2_fourcc('J', '2', 'B', '2')

/* Jetson AGX Xavier and Xavier NX pixel formats */
#define V4L2_PIX_FMT_XAVIER_Y10     v4l2_fourcc('J', 'X', 'Y', '0')
#define V4L2_PIX_FMT_XAVIER_Y12     v4l2_fourcc('J', 'X', 'Y', '2')
#define V4L2_PIX_FMT_XAVIER_SRGGB10 v4l2_fourcc('J', 'X', 'R', '0')
#define V4L2_PIX_FMT_XAVIER_SGRBG10 v4l2_fourcc('J', 'X', 'A', '0')
#define V4L2_PIX_FMT_XAVIER_SGBRG10 v4l2_fourcc('J', 'X', 'G', '0')
#define V4L2_PIX_FMT_XAVIER_SBGGR10 v4l2_fourcc('J', 'X', 'B', '0')
#define V4L2_PIX_FMT_XAVIER_SRGGB12 v4l2_fourcc('J', 'X', 'R', '2')
#define V4L2_PIX_FMT_XAVIER_SGRBG12 v4l2_fourcc('J', 'X', 'A', '2')
#define V4L2_PIX_FMT_XAVIER_SGBRG12 v4l2_fourcc('J', 'X', 'G', '2')
#define V4L2_PIX_FMT_XAVIER_SBGGR12 v4l2_fourcc('J', 'X', 'B', '2')

/* Jetson Nano pixel formats */
#define V4L2_PIX_FMT_NANO_Y10     v4l2_fourcc('J', 'N', 'Y', '0')
#define V4L2_PIX_FMT_NANO_Y12     v4l2_fourcc('J', 'N', 'Y', '2')
#define V4L2_PIX_FMT_NANO_SRGGB10 v4l2_fourcc('J', 'N', 'R', '0')
#define V4L2_PIX_FMT_NANO_SGRBG10 v4l2_fourcc('J', 'N', 'A', '0')
#define V4L2_PIX_FMT_NANO_SGBRG10 v4l2_fourcc('J', 'N', 'G', '0')
#define V4L2_PIX_FMT_NANO_SBGGR10 v4l2_fourcc('J', 'N', 'B', '0')
#define V4L2_PIX_FMT_NANO_SRGGB12 v4l2_fourcc('J', 'N', 'R', '2')
#define V4L2_PIX_FMT_NANO_SGRBG12 v4l2_fourcc('J', 'N', 'A', '2')
#define V4L2_PIX_FMT_NANO_SGBRG12 v4l2_fourcc('J', 'N', 'G', '2')
#define V4L2_PIX_FMT_NANO_SBGGR12 v4l2_fourcc('J', 'N', 'B', '2')



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

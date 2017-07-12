/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ImageTransf.cpp

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

#include <sstream>

#include "ImageTransf.h"
#include "Logger.h"
#include <QPixmap>
#include "videodev2_av.h"

#define CLIP(color) (unsigned char)(((color) > 0xFF) ? 0xff : (((color) < 0) ? 0 : (color)))

namespace AVT {
namespace Tools {


////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////    

ImageTransf::ImageTransf() 
{
}

ImageTransf::~ImageTransf()
{
}

/* inspired by OpenCV's Bayer decoding */
static void v4lconvert_border_bayer8_line_to_bgr24(
		const unsigned char *bayer, const unsigned char *adjacent_bayer,
		unsigned char *bgr, int width, const int start_with_green, const int blue_line)
{
	int t0, t1;

	if (start_with_green) {
		/* First pixel */
		if (blue_line) {
			*bgr++ = bayer[1];
			*bgr++ = bayer[0];
			*bgr++ = adjacent_bayer[0];
		} else {
			*bgr++ = adjacent_bayer[0];
			*bgr++ = bayer[0];
			*bgr++ = bayer[1];
		}
		/* Second pixel */
		t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
		t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
		if (blue_line) {
			*bgr++ = bayer[1];
			*bgr++ = t0;
			*bgr++ = t1;
		} else {
			*bgr++ = t1;
			*bgr++ = t0;
			*bgr++ = bayer[1];
		}
		bayer++;
		adjacent_bayer++;
		width -= 2;
	} else {
		/* First pixel */
		t0 = (bayer[1] + adjacent_bayer[0] + 1) >> 1;
		if (blue_line) {
			*bgr++ = bayer[0];
			*bgr++ = t0;
			*bgr++ = adjacent_bayer[1];
		} else {
			*bgr++ = adjacent_bayer[1];
			*bgr++ = t0;
			*bgr++ = bayer[0];
		}
		width--;
	}

	if (blue_line) {
		for ( ; width > 2; width -= 2) {
			t0 = (bayer[0] + bayer[2] + 1) >> 1;
			*bgr++ = t0;
			*bgr++ = bayer[1];
			*bgr++ = adjacent_bayer[1];
			bayer++;
			adjacent_bayer++;

			t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
			t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
			*bgr++ = bayer[1];
			*bgr++ = t0;
			*bgr++ = t1;
			bayer++;
			adjacent_bayer++;
		}
	} else {
		for ( ; width > 2; width -= 2) {
			t0 = (bayer[0] + bayer[2] + 1) >> 1;
			*bgr++ = adjacent_bayer[1];
			*bgr++ = bayer[1];
			*bgr++ = t0;
			bayer++;
			adjacent_bayer++;

			t0 = (bayer[0] + bayer[2] + adjacent_bayer[1] + 1) / 3;
			t1 = (adjacent_bayer[0] + adjacent_bayer[2] + 1) >> 1;
			*bgr++ = t1;
			*bgr++ = t0;
			*bgr++ = bayer[1];
			bayer++;
			adjacent_bayer++;
		}
	}

	if (width == 2) {
		/* Second to last pixel */
		t0 = (bayer[0] + bayer[2] + 1) >> 1;
		if (blue_line) {
			*bgr++ = t0;
			*bgr++ = bayer[1];
			*bgr++ = adjacent_bayer[1];
		} else {
			*bgr++ = adjacent_bayer[1];
			*bgr++ = bayer[1];
			*bgr++ = t0;
		}
		/* Last pixel */
		t0 = (bayer[1] + adjacent_bayer[2] + 1) >> 1;
		if (blue_line) {
			*bgr++ = bayer[2];
			*bgr++ = t0;
			*bgr++ = adjacent_bayer[1];
		} else {
			*bgr++ = adjacent_bayer[1];
			*bgr++ = t0;
			*bgr++ = bayer[2];
		}
	} else {
		/* Last pixel */
		if (blue_line) {
			*bgr++ = bayer[0];
			*bgr++ = bayer[1];
			*bgr++ = adjacent_bayer[1];
		} else {
			*bgr++ = adjacent_bayer[1];
			*bgr++ = bayer[1];
			*bgr++ = bayer[0];
		}
	}
}

/* From libdc1394, which on turn was based on OpenCV's Bayer decoding */
static void bayer8_to_rgbbgr24(const unsigned char *bayer,
		unsigned char *bgr, int width, int height, const unsigned int stride, unsigned int pixfmt,
		int start_with_green, int blue_line)
{
	/* render the first line */
	v4lconvert_border_bayer8_line_to_bgr24(bayer, bayer + stride, bgr, width,
			start_with_green, blue_line);
	bgr += width * 3;

	/* reduce height by 2 because of the special case top/bottom line */
	for (height -= 2; height; height--) {
		int t0, t1;
		/* (width - 2) because of the border */
		const unsigned char *bayer_end = bayer + (width - 2);

		if (start_with_green) {

			t0 = (bayer[1] + bayer[stride * 2 + 1] + 1) >> 1;
			/* Write first pixel */
			t1 = (bayer[0] + bayer[stride * 2] + bayer[stride + 1] + 1) / 3;
			if (blue_line) {
				*bgr++ = t0;
				*bgr++ = t1;
				*bgr++ = bayer[stride];
			} else {
				*bgr++ = bayer[stride];
				*bgr++ = t1;
				*bgr++ = t0;
			}

			/* Write second pixel */
			t1 = (bayer[stride] + bayer[stride + 2] + 1) >> 1;
			if (blue_line) {
				*bgr++ = t0;
				*bgr++ = bayer[stride + 1];
				*bgr++ = t1;
			} else {
				*bgr++ = t1;
				*bgr++ = bayer[stride + 1];
				*bgr++ = t0;
			}
			bayer++;
		} else {
			/* Write first pixel */
			t0 = (bayer[0] + bayer[stride * 2] + 1) >> 1;
			if (blue_line) {
				*bgr++ = t0;
				*bgr++ = bayer[stride];
				*bgr++ = bayer[stride + 1];
			} else {
				*bgr++ = bayer[stride + 1];
				*bgr++ = bayer[stride];
				*bgr++ = t0;
			}
		}

		if (blue_line) {
			for (; bayer <= bayer_end - 2; bayer += 2) {
				t0 = (bayer[0] + bayer[2] + bayer[stride * 2] +
					bayer[stride * 2 + 2] + 2) >> 2;
				t1 = (bayer[1] + bayer[stride] + bayer[stride + 2] +
					bayer[stride * 2 + 1] + 2) >> 2;
				*bgr++ = t0;
				*bgr++ = t1;
				*bgr++ = bayer[stride + 1];

				t0 = (bayer[2] + bayer[stride * 2 + 2] + 1) >> 1;
				t1 = (bayer[stride + 1] + bayer[stride + 3] + 1) >> 1;
				*bgr++ = t0;
				*bgr++ = bayer[stride + 2];
				*bgr++ = t1;
			}
		} else {
			for (; bayer <= bayer_end - 2; bayer += 2) {
				t0 = (bayer[0] + bayer[2] + bayer[stride * 2] +
					bayer[stride * 2 + 2] + 2) >> 2;
				t1 = (bayer[1] + bayer[stride] + bayer[stride + 2] +
					bayer[stride * 2 + 1] + 2) >> 2;
				*bgr++ = bayer[stride + 1];
				*bgr++ = t1;
				*bgr++ = t0;

				t0 = (bayer[2] + bayer[stride * 2 + 2] + 1) >> 1;
				t1 = (bayer[stride + 1] + bayer[stride + 3] + 1) >> 1;
				*bgr++ = t1;
				*bgr++ = bayer[stride + 2];
				*bgr++ = t0;
			}
		}

		if (bayer < bayer_end) {
			/* write second to last pixel */
			t0 = (bayer[0] + bayer[2] + bayer[stride * 2] +
				bayer[stride * 2 + 2] + 2) >> 2;
			t1 = (bayer[1] + bayer[stride] + bayer[stride + 2] +
				bayer[stride * 2 + 1] + 2) >> 2;
			if (blue_line) {
				*bgr++ = t0;
				*bgr++ = t1;
				*bgr++ = bayer[stride + 1];
			} else {
				*bgr++ = bayer[stride + 1];
				*bgr++ = t1;
				*bgr++ = t0;
			}
			/* write last pixel */
			t0 = (bayer[2] + bayer[stride * 2 + 2] + 1) >> 1;
			if (blue_line) {
				*bgr++ = t0;
				*bgr++ = bayer[stride + 2];
				*bgr++ = bayer[stride + 1];
			} else {
				*bgr++ = bayer[stride + 1];
				*bgr++ = bayer[stride + 2];
				*bgr++ = t0;
			}

			bayer++;

		} else {
			/* write last pixel */
			t0 = (bayer[0] + bayer[stride * 2] + 1) >> 1;
			t1 = (bayer[1] + bayer[stride * 2 + 1] + bayer[stride] + 1) / 3;
			if (blue_line) {
				*bgr++ = t0;
				*bgr++ = t1;
				*bgr++ = bayer[stride + 1];
			} else {
				*bgr++ = bayer[stride + 1];
				*bgr++ = t1;
				*bgr++ = t0;
			}

		}

		/* skip 2 border pixels and padding */
		bayer += (stride - width) + 2;

		blue_line = !blue_line;
		start_with_green = !start_with_green;
	}

	/* render the last line */
	v4lconvert_border_bayer8_line_to_bgr24(bayer + stride, bayer, bgr, width,
			!start_with_green, !blue_line);
}

void v4lconvert_bayer8_to_rgb24(const unsigned char *bayer,
		unsigned char *bgr, int width, int height, const unsigned int stride, unsigned int pixfmt)
{
	bayer8_to_rgbbgr24(bayer, bgr, width, height, stride, pixfmt,
			pixfmt == V4L2_PIX_FMT_SGBRG8		/* start with green */
			|| pixfmt == V4L2_PIX_FMT_SGRBG8,
			pixfmt != V4L2_PIX_FMT_SBGGR8		/* blue line */
			&& pixfmt != V4L2_PIX_FMT_SGBRG8);
}

void v4lconvert_grey_to_rgb24(const unsigned char *src, unsigned char *dest,
		int width, int height)
{
	int j;
	while (--height >= 0) {
		for (j = 0; j < width; j++) {
			*dest++ = *src;
			*dest++ = *src;
			*dest++ = *src;
			src++;
		}
	}
}

void v4lconvert_swap_rgb(const unsigned char *src, unsigned char *dst, 
        int width, int height)
{
	int i;

	for (i = 0; i < (width * height); i++) {
		unsigned char tmp0, tmp1;
		tmp0 = *src++;
		tmp1 = *src++;
		*dst++ = *src++;
		*dst++ = tmp1;
		*dst++ = tmp0;
	}
}

void v4lconvert_uyvy_to_rgb24(const unsigned char *src, unsigned char *dest,
		int width, int height, int stride)
{
	int j;

	while (--height >= 0) {
		for (j = 0; j + 1 < width; j += 2) {
			int u = src[0];
			int v = src[2];
			int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
			int rg = (((u - 128) << 1) +  (u - 128) +
					((v - 128) << 2) + ((v - 128) << 1)) >> 3;
			int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

			*dest++ = CLIP(src[1] + v1);
			*dest++ = CLIP(src[1] - rg);
			*dest++ = CLIP(src[1] + u1);

			*dest++ = CLIP(src[3] + v1);
			*dest++ = CLIP(src[3] - rg);
			*dest++ = CLIP(src[3] + u1);
			src += 4;
		}
		src += stride - width * 2;
	}
}

void v4lconvert_yuyv_to_rgb24(const unsigned char *src, unsigned char *dest,
		int width, int height, int stride)
{
	int j;

	while (--height >= 0) {
		for (j = 0; j + 1 < width; j += 2) {
			int u = src[1];
			int v = src[3];
			int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
			int rg = (((u - 128) << 1) +  (u - 128) +
					((v - 128) << 2) + ((v - 128) << 1)) >> 3;
			int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

			*dest++ = CLIP(src[0] + v1);
			*dest++ = CLIP(src[0] - rg);
			*dest++ = CLIP(src[0] + u1);

			*dest++ = CLIP(src[2] + v1);
			*dest++ = CLIP(src[2] - rg);
			*dest++ = CLIP(src[2] + u1);
			src += 4;
		}
		src += stride - (width * 2);
	}
}

void v4lconvert_yuv420_to_rgb24(const unsigned char *src, unsigned char *dest,
		int width, int height, int yvu)
{
	int i, j;

	const unsigned char *ysrc = src;
	const unsigned char *usrc, *vsrc;

	if (yvu) {
		vsrc = src + width * height;
		usrc = vsrc + (width * height) / 4;
	} else {
		usrc = src + width * height;
		vsrc = usrc + (width * height) / 4;
	}

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j += 2) {
#if 1 /* fast slightly less accurate multiplication free code */
			int u1 = (((*usrc - 128) << 7) +  (*usrc - 128)) >> 6;
			int rg = (((*usrc - 128) << 1) +  (*usrc - 128) +
					((*vsrc - 128) << 2) + ((*vsrc - 128) << 1)) >> 3;
			int v1 = (((*vsrc - 128) << 1) +  (*vsrc - 128)) >> 1;

			*dest++ = CLIP(*ysrc + v1);
			*dest++ = CLIP(*ysrc - rg);
			*dest++ = CLIP(*ysrc + u1);
			ysrc++;

			*dest++ = CLIP(*ysrc + v1);
			*dest++ = CLIP(*ysrc - rg);
			*dest++ = CLIP(*ysrc + u1);
#else
			*dest++ = YUV2R(*ysrc, *usrc, *vsrc);
			*dest++ = YUV2G(*ysrc, *usrc, *vsrc);
			*dest++ = YUV2B(*ysrc, *usrc, *vsrc);
			ysrc++;

			*dest++ = YUV2R(*ysrc, *usrc, *vsrc);
			*dest++ = YUV2G(*ysrc, *usrc, *vsrc);
			*dest++ = YUV2B(*ysrc, *usrc, *vsrc);
#endif
			ysrc++;
			usrc++;
			vsrc++;
		}
		/* Rewind u and v for next line */
		if (!(i&1)) {
			usrc -= width / 2;
			vsrc -= width / 2;
		}
	}
}

void v4lconvert_rgb565_to_rgb24(const unsigned char *src, unsigned char *dest,
		int width, int height)
{
	int j;
	while (--height >= 0) {
		for (j = 0; j < width; j++) {
			unsigned short tmp = *(unsigned short *)src;

			/* Original format: rrrrrggg gggbbbbb */
			*dest++ = 0xf8 & (tmp >> 8);
			*dest++ = 0xfc & (tmp >> 3);
			*dest++ = 0xf8 & (tmp << 3);

			src += 2;
		}
	}
}


int ImageTransf::ConvertFrame(const uint8_t* pBuffer, uint32_t length, 
							  uint32_t width, uint32_t height, uint32_t pixelformat,
							  uint32_t &payloadSize, uint32_t &bytesPerLine, QImage &convertedImage)
{
	int result = 0;
	
	if (NULL == pBuffer || 0 == length)
	    return -1;
	
    switch(pixelformat)
    {
	case V4L2_PIX_FMT_JPEG:
	case V4L2_PIX_FMT_MJPEG:
        {	
            QPixmap pix;
            pix.loadFromData(pBuffer, payloadSize, "JPG");
            convertedImage = pix.toImage();
        }
        break;
    case V4L2_PIX_FMT_RGB565:
        {
            convertedImage = QImage(width, height, QImage::Format_RGB888);
            v4lconvert_rgb565_to_rgb24(pBuffer, convertedImage.bits(), width, height);
        }
        break;
    case V4L2_PIX_FMT_BGR24:
        {
            convertedImage = QImage(width, height, QImage::Format_RGB888);
            v4lconvert_swap_rgb(pBuffer, convertedImage.bits(), width, height);
        }
        break;
	case V4L2_PIX_FMT_UYVY:
        {	
            convertedImage = QImage(width, height, QImage::Format_RGB888);
            v4lconvert_uyvy_to_rgb24(pBuffer, convertedImage.bits(), width, height, bytesPerLine);
        }
        break;
	case V4L2_PIX_FMT_YUYV:
        {	
            convertedImage = QImage(width, height, QImage::Format_RGB888);
            v4lconvert_yuyv_to_rgb24(pBuffer, convertedImage.bits(), width, height, bytesPerLine);
        }
        break;
	case V4L2_PIX_FMT_YUV420:
        {	
            convertedImage = QImage(width, height, QImage::Format_RGB888);
	        v4lconvert_yuv420_to_rgb24(pBuffer, convertedImage.bits(), width, height, 1);
        }
        break;
	case V4L2_PIX_FMT_RGB24:
	{	
            convertedImage = QImage(width, height, QImage::Format_RGB888);
	        memcpy(convertedImage.bits(), pBuffer, width*height*3);
        }
        break;
	case V4L2_PIX_FMT_RGB32:
	case V4L2_PIX_FMT_BGR32:
        {
            convertedImage = QImage(width, height, QImage::Format_RGB32);
	        memcpy(convertedImage.bits(), pBuffer, width*height*4);
        }
        break;
    case V4L2_PIX_FMT_GREY:
        {
            convertedImage = QImage(width, height, QImage::Format_RGB888);
	        v4lconvert_grey_to_rgb24(pBuffer, convertedImage.bits(), width, height);
        }
        break;
    case V4L2_PIX_FMT_SBGGR8:
	case V4L2_PIX_FMT_SGBRG8:
	case V4L2_PIX_FMT_SGRBG8:
	case V4L2_PIX_FMT_SRGGB8:
        {
            convertedImage = QImage(width, height, QImage::Format_RGB888);
            v4lconvert_bayer8_to_rgb24(pBuffer, convertedImage.bits(), width, height, bytesPerLine, pixelformat);
        }
		break;
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SRGGB10:
        return -1;
        break;
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SGBRG12:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SRGGB12:
        return -1;
        break;

    /* L&T */
    /* 10bit raw bayer packed, 5 bytes for every 4 pixels */
    /*case V4L2_PIX_FMT_SBGGR10P:
    case V4L2_PIX_FMT_SGBRG10P:
    case V4L2_PIX_FMT_SGRBG10P:
    case V4L2_PIX_FMT_SRGGB10P:
        return -1;
        break;
*/
    /* 12bit raw bayer packed, 6 bytes for every 4 pixels */
    /*case V4L2_PIX_FMT_SBGGR12P:
    case V4L2_PIX_FMT_SGBRG12P:
    case V4L2_PIX_FMT_SGRBG12P:
    case V4L2_PIX_FMT_SRGGB12P:
        return -1;
        break;
*/
    default:
        return -1;
    }
	
    return result;
}

}} // namespace AVT::Tools

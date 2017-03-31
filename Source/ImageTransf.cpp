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
#include <linux/videodev2.h>

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

int ImageTransf::ConvertFrame(const uint8_t* pBuffer, uint32_t length, 
							  uint32_t width, uint32_t height, uint32_t pixelformat,
							  uint32_t &payloadSize, uint32_t &bytesPerLine, QImage &convertedImage)
{
	int result = 0;
	
	if (NULL == pBuffer || 0 == length)
	    return -1;
	
	if (pixelformat == V4L2_PIX_FMT_JPEG ||
		pixelformat == V4L2_PIX_FMT_MJPEG)
	{	
		QPixmap pix;
		pix.loadFromData(pBuffer, payloadSize, "JPG");
		convertedImage = pix.toImage();
	}
	else if (pixelformat == V4L2_PIX_FMT_UYVY)
	{	
		convertedImage = QImage(width, height, QImage::Format_RGB888);
		v4lconvert_uyvy_to_rgb24(pBuffer, convertedImage.bits(), width, height, bytesPerLine);
	}
	else if (pixelformat == V4L2_PIX_FMT_YUYV)
	{	
		convertedImage = QImage(width, height, QImage::Format_RGB888);
		v4lconvert_yuyv_to_rgb24(pBuffer, convertedImage.bits(), width, height, bytesPerLine);
	}
	else if (pixelformat == V4L2_PIX_FMT_YUV420)
	{	
		convertedImage = QImage(width, height, QImage::Format_RGB888);
	        v4lconvert_yuv420_to_rgb24(pBuffer, convertedImage.bits(), width, height, 1);
	}	
	else if (pixelformat == V4L2_PIX_FMT_RGB24 ||
			 pixelformat == V4L2_PIX_FMT_BGR24)
	{	
		convertedImage = QImage(width, height, QImage::Format_RGB888);
	        memcpy(convertedImage.bits(), pBuffer, width*height*3);
	}
	else if (pixelformat == V4L2_PIX_FMT_RGB32 ||
			 pixelformat == V4L2_PIX_FMT_BGR32)
	{
		convertedImage = QImage(width, height, QImage::Format_RGB32);
	        memcpy(convertedImage.bits(), pBuffer, width*height*4);
	}
	else
	{
		return -1;
	}
	
	return result;
}

}} // namespace AVT::Tools

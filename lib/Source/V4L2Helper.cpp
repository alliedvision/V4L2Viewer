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


#include "videodev2_av.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <V4L2Helper.h>

#include <iomanip>
#include <sstream>

namespace v4l2helper {

std::string ConvertPixelFormat2EnumString(int pixelFormat)
{
    switch(pixelFormat)
    {
        case V4L2_PIX_FMT_ABGR32: return "V4L2_PIX_FMT_ABGR32";
        case V4L2_PIX_FMT_XBGR32: return "V4L2_PIX_FMT_XBGR32";
        case V4L2_PIX_FMT_XRGB32: return "V4L2_PIX_FMT_XRGB32";

        /* RGB formats */
        case V4L2_PIX_FMT_RGB332: return "V4L2_PIX_FMT_RGB332";
        case V4L2_PIX_FMT_RGB444: return "V4L2_PIX_FMT_RGB444";
        case V4L2_PIX_FMT_RGB555: return "V4L2_PIX_FMT_RGB555";
        case V4L2_PIX_FMT_RGB565: return "V4L2_PIX_FMT_RGB565";
        case V4L2_PIX_FMT_RGB555X: return "V4L2_PIX_FMT_RGB555X";
        case V4L2_PIX_FMT_RGB565X: return "V4L2_PIX_FMT_RGB565X";
        case V4L2_PIX_FMT_BGR666: return "V4L2_PIX_FMT_BGR666";
        case V4L2_PIX_FMT_BGR24: return "V4L2_PIX_FMT_BGR24";
        case V4L2_PIX_FMT_RGB24: return "V4L2_PIX_FMT_RGB24";
        case V4L2_PIX_FMT_BGR32: return "V4L2_PIX_FMT_BGR32";
        case V4L2_PIX_FMT_RGB32: return "V4L2_PIX_FMT_RGB32";

        /* Grey formats */
        case V4L2_PIX_FMT_GREY: return "V4L2_PIX_FMT_GREY";
        case V4L2_PIX_FMT_Y4: return "V4L2_PIX_FMT_Y4";
        case V4L2_PIX_FMT_Y6: return "V4L2_PIX_FMT_Y6";
        case V4L2_PIX_FMT_Y10: return "V4L2_PIX_FMT_Y10";
        case V4L2_PIX_FMT_Y12: return "V4L2_PIX_FMT_Y12";
        case V4L2_PIX_FMT_Y16: return "V4L2_PIX_FMT_Y16";

        /* Grey bit-packed formats */
        case V4L2_PIX_FMT_Y10BPACK: return "V4L2_PIX_FMT_Y10BPACK";

        /* Palette formats */
        case V4L2_PIX_FMT_PAL8: return "V4L2_PIX_FMT_PAL8";

        /* Chrominance formats */
        case V4L2_PIX_FMT_UV8: return "V4L2_PIX_FMT_UV8";

        /* Luminance+Chrominance formats */
        case V4L2_PIX_FMT_YVU410: return "V4L2_PIX_FMT_YVU410";
        case V4L2_PIX_FMT_YVU420: return "V4L2_PIX_FMT_YVU420";
        case V4L2_PIX_FMT_YUYV: return "V4L2_PIX_FMT_YUYV";
        case V4L2_PIX_FMT_YYUV: return "V4L2_PIX_FMT_YYUV";
        case V4L2_PIX_FMT_YVYU: return "V4L2_PIX_FMT_YVYU";
        case V4L2_PIX_FMT_UYVY: return "V4L2_PIX_FMT_UYVY";
        case V4L2_PIX_FMT_VYUY: return "V4L2_PIX_FMT_VYUY";
        case V4L2_PIX_FMT_YUV422P: return "V4L2_PIX_FMT_YUV422P";
        case V4L2_PIX_FMT_YUV411P: return "V4L2_PIX_FMT_YUV411P";
        case V4L2_PIX_FMT_Y41P: return "V4L2_PIX_FMT_Y41P";
        case V4L2_PIX_FMT_YUV444: return "V4L2_PIX_FMT_YUV444";
        case V4L2_PIX_FMT_YUV555: return "V4L2_PIX_FMT_YUV555";
        case V4L2_PIX_FMT_YUV565: return "V4L2_PIX_FMT_YUV565";
        case V4L2_PIX_FMT_YUV32: return "V4L2_PIX_FMT_YUV32";
        case V4L2_PIX_FMT_YUV410: return "V4L2_PIX_FMT_YUV410";
        case V4L2_PIX_FMT_YUV420: return "V4L2_PIX_FMT_YUV420";
        case V4L2_PIX_FMT_HI240: return "V4L2_PIX_FMT_HI240";
        case V4L2_PIX_FMT_HM12: return "V4L2_PIX_FMT_HM12";
        case V4L2_PIX_FMT_M420: return "V4L2_PIX_FMT_M420";

        /* two planes -- one Y, one Cr + Cb interleaved  */
        case V4L2_PIX_FMT_NV12: return "V4L2_PIX_FMT_NV12";
        case V4L2_PIX_FMT_NV21: return "V4L2_PIX_FMT_NV21";
        case V4L2_PIX_FMT_NV16: return "V4L2_PIX_FMT_NV16";
        case V4L2_PIX_FMT_NV61: return "V4L2_PIX_FMT_NV61";
        case V4L2_PIX_FMT_NV24: return "V4L2_PIX_FMT_NV24";
        case V4L2_PIX_FMT_NV42: return "V4L2_PIX_FMT_NV42";

        /* two non contiguous planes - one Y, one Cr + Cb interleaved  */
        case V4L2_PIX_FMT_NV12M: return "V4L2_PIX_FMT_NV12M";
        case V4L2_PIX_FMT_NV21M: return "V4L2_PIX_FMT_NV21M";
        case V4L2_PIX_FMT_NV16M: return "V4L2_PIX_FMT_NV16M";
        case V4L2_PIX_FMT_NV61M: return "V4L2_PIX_FMT_NV61M";
        case V4L2_PIX_FMT_NV12MT: return "V4L2_PIX_FMT_NV12MT";
        case V4L2_PIX_FMT_NV12MT_16X16: return "V4L2_PIX_FMT_NV12MT_16X16";

        /* three non contiguous planes - Y, Cb, Cr */
        case V4L2_PIX_FMT_YUV420M: return "V4L2_PIX_FMT_YUV420M";
        case V4L2_PIX_FMT_YVU420M: return "V4L2_PIX_FMT_YVU420M";

        /* Bayer formats - see http://www.siliconimaging.com/RGB%20Bayer.htm */
        case V4L2_PIX_FMT_SBGGR8: return "V4L2_PIX_FMT_SBGGR8";
        case V4L2_PIX_FMT_SGBRG8: return "V4L2_PIX_FMT_SGBRG8";
        case V4L2_PIX_FMT_SGRBG8: return "V4L2_PIX_FMT_SGRBG8";
        case V4L2_PIX_FMT_SRGGB8: return "V4L2_PIX_FMT_SRGGB8";
        case V4L2_PIX_FMT_SBGGR10: return "V4L2_PIX_FMT_SBGGR10";
        case V4L2_PIX_FMT_SGBRG10: return "V4L2_PIX_FMT_SGBRG10";
        case V4L2_PIX_FMT_SGRBG10: return "V4L2_PIX_FMT_SGRBG10";
        case V4L2_PIX_FMT_SRGGB10: return "V4L2_PIX_FMT_SRGGB10";
        case V4L2_PIX_FMT_SBGGR12: return "V4L2_PIX_FMT_SBGGR12";
        case V4L2_PIX_FMT_SGBRG12: return "V4L2_PIX_FMT_SGBRG12";
        case V4L2_PIX_FMT_SGRBG12: return "V4L2_PIX_FMT_SGRBG12";
        case V4L2_PIX_FMT_SRGGB12: return "V4L2_PIX_FMT_SRGGB12";
        /* 10bit raw bayer a-law compressed to 8 bits */
        case V4L2_PIX_FMT_SBGGR10ALAW8: return "V4L2_PIX_FMT_SBGGR10ALAW8";
        case V4L2_PIX_FMT_SGBRG10ALAW8: return "V4L2_PIX_FMT_SGBRG10ALAW8";
        case V4L2_PIX_FMT_SGRBG10ALAW8: return "V4L2_PIX_FMT_SGRBG10ALAW8";
        case V4L2_PIX_FMT_SRGGB10ALAW8: return "V4L2_PIX_FMT_SRGGB10ALAW8";
        /* 10bit raw bayer DPCM compressed to 8 bits */
        case V4L2_PIX_FMT_SBGGR10DPCM8: return "V4L2_PIX_FMT_SBGGR10DPCM8";
        case V4L2_PIX_FMT_SGBRG10DPCM8: return "V4L2_PIX_FMT_SGBRG10DPCM8";
        case V4L2_PIX_FMT_SGRBG10DPCM8: return "V4L2_PIX_FMT_SGRBG10DPCM8";
        case V4L2_PIX_FMT_SRGGB10DPCM8: return "V4L2_PIX_FMT_SRGGB10DPCM8";

        case V4L2_PIX_FMT_SBGGR16: return "V4L2_PIX_FMT_SBGGR16";

        /* compressed formats */
        case V4L2_PIX_FMT_MJPEG: return "V4L2_PIX_FMT_MJPEG";
        case V4L2_PIX_FMT_JPEG: return "V4L2_PIX_FMT_JPEG";
        case V4L2_PIX_FMT_DV: return "V4L2_PIX_FMT_DV";
        case V4L2_PIX_FMT_MPEG: return "V4L2_PIX_FMT_MPEG";
        case V4L2_PIX_FMT_H264: return "V4L2_PIX_FMT_H264";
        case V4L2_PIX_FMT_H264_NO_SC: return "V4L2_PIX_FMT_H264_NO_SC";
        case V4L2_PIX_FMT_H264_MVC: return "V4L2_PIX_FMT_H264_MVC";
        case V4L2_PIX_FMT_H263: return "V4L2_PIX_FMT_H263";
        case V4L2_PIX_FMT_MPEG1: return "V4L2_PIX_FMT_MPEG1";
        case V4L2_PIX_FMT_MPEG2: return "V4L2_PIX_FMT_MPEG2";
        case V4L2_PIX_FMT_MPEG4: return "V4L2_PIX_FMT_MPEG4";
        case V4L2_PIX_FMT_XVID: return "V4L2_PIX_FMT_XVID";
        case V4L2_PIX_FMT_VC1_ANNEX_G: return "V4L2_PIX_FMT_VC1_ANNEX_G";
        case V4L2_PIX_FMT_VC1_ANNEX_L: return "V4L2_PIX_FMT_VC1_ANNEX_L";
        case V4L2_PIX_FMT_VP8: return "V4L2_PIX_FMT_VP8";

        /*  Vendor-specific formats   */
        case V4L2_PIX_FMT_CPIA1: return "V4L2_PIX_FMT_CPIA1";
        case V4L2_PIX_FMT_WNVA: return "V4L2_PIX_FMT_WNVA";
        case V4L2_PIX_FMT_SN9C10X: return "V4L2_PIX_FMT_SN9C10X";
        case V4L2_PIX_FMT_SN9C20X_I420: return "V4L2_PIX_FMT_SN9C20X_I420";
        case V4L2_PIX_FMT_PWC1: return "V4L2_PIX_FMT_PWC1";
        case V4L2_PIX_FMT_PWC2: return "V4L2_PIX_FMT_PWC2";
        case V4L2_PIX_FMT_ET61X251: return "V4L2_PIX_FMT_ET61X251";
        case V4L2_PIX_FMT_SPCA501: return "V4L2_PIX_FMT_SPCA501";
        case V4L2_PIX_FMT_SPCA505: return "V4L2_PIX_FMT_SPCA505";
        case V4L2_PIX_FMT_SPCA508: return "V4L2_PIX_FMT_SPCA508";
        case V4L2_PIX_FMT_SPCA561: return "V4L2_PIX_FMT_SPCA561";
        case V4L2_PIX_FMT_PAC207: return "V4L2_PIX_FMT_PAC207";
        case V4L2_PIX_FMT_MR97310A: return "V4L2_PIX_FMT_MR97310A";
        case V4L2_PIX_FMT_JL2005BCD: return "V4L2_PIX_FMT_JL2005BCD";
        case V4L2_PIX_FMT_SN9C2028: return "V4L2_PIX_FMT_SN9C2028";
        case V4L2_PIX_FMT_SQ905C: return "V4L2_PIX_FMT_SQ905C";
        case V4L2_PIX_FMT_PJPG: return "V4L2_PIX_FMT_PJPG";
        case V4L2_PIX_FMT_OV511: return "V4L2_PIX_FMT_OV511";
        case V4L2_PIX_FMT_OV518: return "V4L2_PIX_FMT_OV511";
        case V4L2_PIX_FMT_STV0680: return "V4L2_PIX_FMT_STV0680";
        case V4L2_PIX_FMT_TM6000: return "V4L2_PIX_FMT_TM6000";
        case V4L2_PIX_FMT_CIT_YYVYUY: return "V4L2_PIX_FMT_CIT_YYVYUY";
        case V4L2_PIX_FMT_KONICA420: return "V4L2_PIX_FMT_KONICA420";
        case V4L2_PIX_FMT_JPGL: return "V4L2_PIX_FMT_JPGL";
        case V4L2_PIX_FMT_SE401: return "V4L2_PIX_FMT_SE401";
        case V4L2_PIX_FMT_S5C_UYVY_JPG: return "V4L2_PIX_FMT_S5C_UYVY_JPG";

        /* 10bit and 12bit greyscale packed */
        case V4L2_PIX_FMT_Y10P: return "V4L2_PIX_FMT_Y10P";
        case V4L2_PIX_FMT_Y12P: return "V4L2_PIX_FMT_Y12P";

        /* 10bit raw bayer packed, 5 bytes for every 4 pixels */
        case V4L2_PIX_FMT_SBGGR10P: return "V4L2_PIX_FMT_SBGGR10P";
        case V4L2_PIX_FMT_SGBRG10P: return "V4L2_PIX_FMT_SGBRG10P";
        case V4L2_PIX_FMT_SGRBG10P: return "V4L2_PIX_FMT_SGRBG10P";
        case V4L2_PIX_FMT_SRGGB10P: return "V4L2_PIX_FMT_SRGGB10P";

        /* 12bit raw bayer packed, 6 bytes for every 4 pixels */
        case V4L2_PIX_FMT_SBGGR12P: return "V4L2_PIX_FMT_SBGGR12P";
        case V4L2_PIX_FMT_SGBRG12P: return "V4L2_PIX_FMT_SGBRG12P";
        case V4L2_PIX_FMT_SGRBG12P: return "V4L2_PIX_FMT_SGRBG12P";
        case V4L2_PIX_FMT_SRGGB12P: return "V4L2_PIX_FMT_SRGGB12P";

        /* 10 and 12 bit jetson formats */
        case V4L2_PIX_FMT_XAVIER_Y10: return "V4L2_PIX_FMT_XAVIER_Y10";
        case V4L2_PIX_FMT_XAVIER_Y12: return "V4L2_PIX_FMT_XAVIER_Y12";
        case V4L2_PIX_FMT_XAVIER_SGRBG10: return "V4L2_PIX_FMT_XAVIER_SGRBG10";
        case V4L2_PIX_FMT_XAVIER_SGRBG12: return "V4L2_PIX_FMT_XAVIER_SGRBG12";
        case V4L2_PIX_FMT_XAVIER_SRGGB10: return "V4L2_PIX_FMT_XAVIER_SRGGB10";
        case V4L2_PIX_FMT_XAVIER_SRGGB12: return "V4L2_PIX_FMT_XAVIER_SRGGB12";
        case V4L2_PIX_FMT_XAVIER_SGBRG10: return "V4L2_PIX_FMT_XAVIER_SGBRG10";
        case V4L2_PIX_FMT_XAVIER_SGBRG12: return "V4L2_PIX_FMT_XAVIER_SGBRG12";
        case V4L2_PIX_FMT_XAVIER_SBGGR10: return "V4L2_PIX_FMT_XAVIER_SBGGR10";
        case V4L2_PIX_FMT_XAVIER_SBGGR12: return "V4L2_PIX_FMT_XAVIER_SBGGR12";
        case V4L2_PIX_FMT_TX2_Y10: return "V4L2_PIX_FMT_TX2_Y10";
        case V4L2_PIX_FMT_TX2_Y12: return "V4L2_PIX_FMT_TX2_Y12";
        case V4L2_PIX_FMT_NANO_Y10: return "V4L2_PIX_FMT_NANO_Y10";
        case V4L2_PIX_FMT_NANO_Y12: return "V4L2_PIX_FMT_NANO_Y12";
        case V4L2_PIX_FMT_TX2_SGRBG10: return "V4L2_PIX_FMT_TX2_SGRBG10";
        case V4L2_PIX_FMT_TX2_SGRBG12: return "V4L2_PIX_FMT_TX2_SGRBG12";
        case V4L2_PIX_FMT_NANO_SGRBG10: return "V4L2_PIX_FMT_NANO_SGRBG10";
        case V4L2_PIX_FMT_NANO_SGRBG12: return "V4L2_PIX_FMT_NANO_SGRBG12";
        case V4L2_PIX_FMT_TX2_SRGGB10: return "V4L2_PIX_FMT_TX2_SRGGB10";
        case V4L2_PIX_FMT_TX2_SRGGB12: return "V4L2_PIX_FMT_TX2_SRGGB12";
        case V4L2_PIX_FMT_NANO_SRGGB10: return "V4L2_PIX_FMT_NANO_SRGGB10";
        case V4L2_PIX_FMT_NANO_SRGGB12: return "V4L2_PIX_FMT_NANO_SRGGB12";
        case V4L2_PIX_FMT_TX2_SGBRG10: return "V4L2_PIX_FMT_TX2_SGBRG10";
        case V4L2_PIX_FMT_TX2_SGBRG12: return "V4L2_PIX_FMT_TX2_SGBRG12";
        case V4L2_PIX_FMT_NANO_SGBRG10: return "V4L2_PIX_FMT_NANO_SGBRG10";
        case V4L2_PIX_FMT_NANO_SGBRG12: return "V4L2_PIX_FMT_NANO_SGBRG12";
        case V4L2_PIX_FMT_TX2_SBGGR10: return "V4L2_PIX_FMT_TX2_SBGGR10";
        case V4L2_PIX_FMT_TX2_SBGGR12: return "V4L2_PIX_FMT_TX2_SBGGR12";
        case V4L2_PIX_FMT_NANO_SBGGR10: return "V4L2_PIX_FMT_NANO_SBGGR10";
        case V4L2_PIX_FMT_NANO_SBGGR12: return "V4L2_PIX_FMT_NANO_SBGGR12";

        default: return "<unknown>";
    }
}

std::string ConvertControlID2String(uint32_t controlID)
{
    std::string result = "";

    switch(controlID)
    {
        // V4L2_CID_USER_CLASS_BASE
        case V4L2_CID_BRIGHTNESS:     result = "V4L2_CID_BRIGHTNESS"; break;
        case V4L2_CID_CONTRAST:     result = "V4L2_CID_CONTRAST"; break;
        case V4L2_CID_SATURATION:     result = "V4L2_CID_SATURATION"; break;
        case V4L2_CID_HUE:         result = "V4L2_CID_HUE"; break;
        case V4L2_CID_AUDIO_VOLUME:     result = "V4L2_CID_AUDIO_VOLUME"; break;
        case V4L2_CID_AUDIO_BALANCE:     result = "V4L2_CID_AUDIO_BALANCE"; break;
        case V4L2_CID_AUDIO_BASS:     result = "V4L2_CID_AUDIO_BASS"; break;
        case V4L2_CID_AUDIO_TREBLE:     result = "V4L2_CID_AUDIO_TREBLE"; break;
        case V4L2_CID_AUDIO_MUTE:     result = "V4L2_CID_AUDIO_MUTE"; break;
        case V4L2_CID_AUDIO_LOUDNESS:     result = "V4L2_CID_AUDIO_LOUDNESS"; break;
        case V4L2_CID_BLACK_LEVEL:     result = "V4L2_CID_BLACK_LEVEL"; break; //deprecated
        case V4L2_CID_AUTO_WHITE_BALANCE:     result = "V4L2_CID_AUTO_WHITE_BALANCE"; break;
        case V4L2_CID_DO_WHITE_BALANCE:     result = "V4L2_CID_DO_WHITE_BALANCE"; break;
        case V4L2_CID_RED_BALANCE:     result = "V4L2_CID_RED_BALANCE"; break;
        case V4L2_CID_BLUE_BALANCE:     result = "V4L2_CID_BLUE_BALANCE"; break;
        case V4L2_CID_GAMMA:     result = "V4L2_CID_GAMMA"; break;
        case V4L2_CID_EXPOSURE:     result = "V4L2_CID_EXPOSURE"; break;
        case V4L2_CID_AUTOGAIN:     result = "V4L2_CID_AUTOGAIN"; break;
        case V4L2_CID_GAIN:     result = "V4L2_CID_GAIN"; break;
        case V4L2_CID_HFLIP:     result = "V4L2_CID_HFLIP"; break;
        case V4L2_CID_VFLIP:     result = "V4L2_CID_VFLIP"; break;
        case V4L2_CID_POWER_LINE_FREQUENCY:     result = "V4L2_CID_POWER_LINE_FREQUENCY"; break;
        case V4L2_CID_HUE_AUTO:     result = "V4L2_CID_HUE_AUTO"; break;
        case V4L2_CID_WHITE_BALANCE_TEMPERATURE:     result = "V4L2_CID_WHITE_BALANCE_TEMPERATURE"; break;
        case V4L2_CID_SHARPNESS:     result = "V4L2_CID_SHARPNESS"; break;
        case V4L2_CID_BACKLIGHT_COMPENSATION:     result = "V4L2_CID_BACKLIGHT_COMPENSATION"; break;
        case V4L2_CID_CHROMA_AGC:     result = "V4L2_CID_CHROMA_AGC"; break;
        case V4L2_CID_COLOR_KILLER:     result = "V4L2_CID_COLOR_KILLER"; break;
        case V4L2_CID_COLORFX:     result = "V4L2_CID_COLORFX"; break;
        case V4L2_CID_AUTOBRIGHTNESS:     result = "V4L2_CID_AUTOBRIGHTNESS"; break;
        case V4L2_CID_BAND_STOP_FILTER:     result = "V4L2_CID_BAND_STOP_FILTER"; break;
        case V4L2_CID_ROTATE:     result = "V4L2_CID_ROTATE"; break;
        case V4L2_CID_BG_COLOR:     result = "V4L2_CID_BG_COLOR"; break;
        case V4L2_CID_CHROMA_GAIN:     result = "V4L2_CID_CHROMA_GAIN"; break;
        case V4L2_CID_ILLUMINATORS_1:     result = "V4L2_CID_ILLUMINATORS_1"; break;
        case V4L2_CID_ILLUMINATORS_2:     result = "V4L2_CID_ILLUMINATORS_2"; break;
        case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:     result = "V4L2_CID_MIN_BUFFERS_FOR_CAPTURE"; break;
        case V4L2_CID_MIN_BUFFERS_FOR_OUTPUT:     result = "V4L2_CID_MIN_BUFFERS_FOR_OUTPUT"; break;
        case V4L2_CID_ALPHA_COMPONENT:     result = "V4L2_CID_ALPHA_COMPONENT"; break;
        case V4L2_CID_COLORFX_CBCR:     result = "V4L2_CID_COLORFX_CBCR"; break;
        case V4L2_CID_LASTP1:     result = "V4L2_CID_LASTP1"; break;

        // V4L2_CID_CAMERA_CLASS_BASE
        case V4L2_CID_EXPOSURE_AUTO:     result = "V4L2_CID_EXPOSURE_AUTO"; break;
        case V4L2_CID_EXPOSURE_ABSOLUTE:     result = "V4L2_CID_EXPOSURE_ABSOLUTE"; break;
        case V4L2_CID_EXPOSURE_AUTO_PRIORITY:     result = "V4L2_CID_EXPOSURE_AUTO_PRIORITY"; break;
        case V4L2_CID_PAN_RELATIVE:     result = "V4L2_CID_PAN_RELATIVE"; break;
        case V4L2_CID_TILT_RELATIVE:     result = "V4L2_CID_TILT_RELATIVE"; break;
        case V4L2_CID_PAN_RESET:     result = "V4L2_CID_PAN_RESET"; break;
        case V4L2_CID_TILT_RESET:     result = "V4L2_CID_TILT_RESET"; break;
        case V4L2_CID_PAN_ABSOLUTE:     result = "V4L2_CID_PAN_ABSOLUTE"; break;
        case V4L2_CID_TILT_ABSOLUTE:     result = "V4L2_CID_TILT_ABSOLUTE"; break;
        case V4L2_CID_FOCUS_ABSOLUTE:     result = "V4L2_CID_FOCUS_ABSOLUTE"; break;
        case V4L2_CID_FOCUS_RELATIVE:     result = "V4L2_CID_FOCUS_RELATIVE"; break;
        case V4L2_CID_FOCUS_AUTO:     result = "V4L2_CID_FOCUS_AUTO"; break;
        case V4L2_CID_ZOOM_ABSOLUTE:     result = "V4L2_CID_ZOOM_ABSOLUTE"; break;
        case V4L2_CID_ZOOM_RELATIVE:     result = "V4L2_CID_ZOOM_RELATIVE"; break;
        case V4L2_CID_ZOOM_CONTINUOUS:     result = "V4L2_CID_ZOOM_CONTINUOUS"; break;
        case V4L2_CID_PRIVACY:     result = "V4L2_CID_PRIVACY"; break;
        case V4L2_CID_IRIS_ABSOLUTE:     result = "V4L2_CID_IRIS_ABSOLUTE"; break;
        case V4L2_CID_IRIS_RELATIVE:     result = "V4L2_CID_IRIS_RELATIVE"; break;
        case V4L2_CID_AUTO_EXPOSURE_BIAS:     result = "V4L2_CID_AUTO_EXPOSURE_BIAS"; break;
        case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:     result = "V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE"; break;
        case V4L2_CID_WIDE_DYNAMIC_RANGE:     result = "V4L2_CID_WIDE_DYNAMIC_RANGE"; break;
        case V4L2_CID_IMAGE_STABILIZATION:     result = "V4L2_CID_IMAGE_STABILIZATION"; break;
        case V4L2_CID_ISO_SENSITIVITY:     result = "V4L2_CID_ISO_SENSITIVITY"; break;
        case V4L2_CID_ISO_SENSITIVITY_AUTO:     result = "V4L2_CID_ISO_SENSITIVITY_AUTO"; break;
        case V4L2_CID_EXPOSURE_METERING:     result = "V4L2_CID_EXPOSURE_METERING"; break;
        case V4L2_CID_SCENE_MODE:     result = "V4L2_CID_SCENE_MODE"; break;
        case V4L2_CID_3A_LOCK:     result = "V4L2_CID_3A_LOCK"; break;
        case V4L2_LOCK_EXPOSURE:     result = "V4L2_LOCK_EXPOSURE"; break;
        case V4L2_LOCK_WHITE_BALANCE:     result = "V4L2_LOCK_WHITE_BALANCE"; break;
        case V4L2_LOCK_FOCUS:     result = "V4L2_LOCK_FOCUS"; break;
        case V4L2_CID_AUTO_FOCUS_START:     result = "V4L2_CID_AUTO_FOCUS_START"; break;
        case V4L2_CID_AUTO_FOCUS_STOP:     result = "V4L2_CID_AUTO_FOCUS_STOP"; break;
        case V4L2_CID_AUTO_FOCUS_STATUS:     result = "V4L2_CID_AUTO_FOCUS_STATUS"; break;
        case V4L2_CID_AUTO_FOCUS_RANGE:     result = "V4L2_CID_AUTO_FOCUS_RANGE"; break;
        default: result = "<unknown>"; break;
    }

    return result;
}

std::string GetControlUnit(int32_t id)
{
    std::string res = "";
    switch (id)
    {
        case V4L2_CID_EXPOSURE: res = "Nanoseconds [ns]"; break;
        case V4L2_CID_EXPOSURE_ABSOLUTE: res = "100*Microseconds [100µs]"; break;
        case V4L2_CID_GAIN: res = "Millibel (1/100 Decibel)"; break;
        case V4L2_CID_HUE: res = "1/100 degrees [°]"; break;
        default: res = "Not Applicable"; break;
    }
    return res;
}



std::string ConvertErrno2String(int errnumber)
{
    std::string result;

    switch(errnumber)
    {
        case EPERM: result = "EPERM=Operation not permitted"; break;
        case ENOENT: result = "ENOENT=No such file or directory"; break;
        case ESRCH: result = "ESRCH=No such process"; break;
        case EINTR: result = "EINTR=Interrupted system call"; break;
        case EIO: result = "EIO=I/O error"; break;
        case ENXIO: result = "ENXIO=No such device or address"; break;
        case E2BIG: result = "E2BIG=Argument list too long"; break;
        case ENOEXEC: result = "ENOEXEC=Exec format error"; break;
        case EBADF: result = "EBADF=Bad file number"; break;
        case ECHILD: result = "ECHILD=No child processes"; break;
        case EAGAIN: result = "EAGAIN=Try again"; break;
        case ENOMEM: result = "ENOMEM=Out of memory"; break;
        case EACCES: result = "EACCES=Permission denied"; break;
        case EFAULT: result = "EFAULT=Bad address"; break;
        case ENOTBLK: result = "ENOTBLK=Block device required"; break;
        case EBUSY: result = "EBUSY=Device or resource busy"; break;
        case EEXIST: result = "EEXIST=File exists"; break;
        case EXDEV: result = "EXDEV=Cross-device link"; break;
        case ENODEV: result = "ENODEV=No such device"; break;
        case ENOTDIR: result = "ENOTDIR=Not a directory"; break;
        case EISDIR: result = "EISDIR=Is a directory"; break;
        case EINVAL: result = "EINVAL=Invalid argument"; break;
        case ENFILE: result = "ENFILE=File table overflow"; break;
        case EMFILE: result = "EMFILE=Too many open files"; break;
        case ENOTTY: result = "ENOTTY=Not a typewriter"; break;
        case ETXTBSY: result = "ETXTBSY=Text file busy"; break;
        case EFBIG: result = "EFBIG=File too large"; break;
        case ENOSPC: result = "ENOSPC=No space left on device"; break;
        case ESPIPE: result = "ESPIPE=Illegal seek"; break;
        case EROFS: result = "EROFS=Read-only file system"; break;
        case EMLINK: result = "EMLINK=Too many links"; break;
        case EPIPE: result = "EPIPE=Broken pipe"; break;
        case EDOM: result = "EDOM=Math argument out of domain of func"; break;
        case ERANGE: result = "ERANGE=Math result not representable"; break;
        default: result = "<unknown>"; break;
    }

    return result;
}

std::string ConvertPixelFormat2String(int pixelFormat)
{
    std::string s;

    s += pixelFormat & 0xff;
    s += (pixelFormat >> 8) & 0xff;
    s += (pixelFormat >> 16) & 0xff;
    s += (pixelFormat >> 24) & 0xff;

    return s;
}

} // namespace v4l2helper

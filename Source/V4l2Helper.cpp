#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <iomanip>
#include <sstream>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "videodev2_av.h"

#include "V4l2Helper.h"

#ifdef _WIN32       /* Windows */
#include <Windows.h>
#else               /* Linux */
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace AVT {
namespace Tools {
namespace Examples {
	
    V4l2Helper::V4l2Helper(void)
    {
    }


    V4l2Helper::~V4l2Helper(void)
    {
    }

    int V4l2Helper::xioctl(int fh, int request, void *arg)
	{
		int result = 0;

		do 
		{
			result = ioctl(fh, request, arg);
		} 
		while (-1 == result && EINTR == errno);

		return result;
	}

	std::string V4l2Helper::ConvertPixelformat2String(int pixelformat)
	{
		std::string s;
		
		s += pixelformat & 0xff;
		s += (pixelformat >> 8) & 0xff;
		s += (pixelformat >> 16) & 0xff;
		s += (pixelformat >> 24) & 0xff;
		
		return s;
	}
	
	std::string V4l2Helper::ConvertPixelformat2EnumString(int pixelformat)
	{
		std:: string result;
		
		switch(pixelformat)
		{
			case V4L2_PIX_FMT_ABGR32: result = "V4L2_PIX_FMT_ABGR32"; break;
            case V4L2_PIX_FMT_XBGR32: result = "V4L2_PIX_FMT_XBGR32"; break;
            case V4L2_PIX_FMT_XRGB32: result = "V4L2_PIX_FMT_XRGB32"; break;

			/* RGB formats */
			case V4L2_PIX_FMT_RGB332: result = "V4L2_PIX_FMT_RGB332"; break;
			case V4L2_PIX_FMT_RGB444: result = "V4L2_PIX_FMT_RGB444"; break;
			case V4L2_PIX_FMT_RGB555: result = "V4L2_PIX_FMT_RGB555"; break;
			case V4L2_PIX_FMT_RGB565: result = "V4L2_PIX_FMT_RGB565"; break;
			case V4L2_PIX_FMT_RGB555X: result = "V4L2_PIX_FMT_RGB555X"; break;
			case V4L2_PIX_FMT_RGB565X: result = "V4L2_PIX_FMT_RGB565X"; break;
			case V4L2_PIX_FMT_BGR666: result = "V4L2_PIX_FMT_BGR666"; break;
			case V4L2_PIX_FMT_BGR24: result = "V4L2_PIX_FMT_BGR24"; break;
			case V4L2_PIX_FMT_RGB24: result = "V4L2_PIX_FMT_RGB24"; break;
			case V4L2_PIX_FMT_BGR32: result = "V4L2_PIX_FMT_BGR32"; break;
			case V4L2_PIX_FMT_RGB32: result = "V4L2_PIX_FMT_RGB32"; break;

			/* Grey formats */
			case V4L2_PIX_FMT_GREY: result = "V4L2_PIX_FMT_GREY"; break;
			case V4L2_PIX_FMT_Y4: result = "V4L2_PIX_FMT_Y4"; break;
			case V4L2_PIX_FMT_Y6: result = "V4L2_PIX_FMT_Y6"; break;
			case V4L2_PIX_FMT_Y10: result = "V4L2_PIX_FMT_Y10"; break;
			case V4L2_PIX_FMT_Y12: result = "V4L2_PIX_FMT_Y12"; break;
			case V4L2_PIX_FMT_Y16: result = "V4L2_PIX_FMT_Y16"; break;

			/* Grey bit-packed formats */
			case V4L2_PIX_FMT_Y10BPACK: result = "V4L2_PIX_FMT_Y10BPACK"; break;

			/* Palette formats */
			case V4L2_PIX_FMT_PAL8: result = "V4L2_PIX_FMT_PAL8"; break;

			/* Chrominance formats */
			case V4L2_PIX_FMT_UV8: result = "V4L2_PIX_FMT_UV8"; break;

			/* Luminance+Chrominance formats */
			case V4L2_PIX_FMT_YVU410: result = "V4L2_PIX_FMT_YVU410"; break;
			case V4L2_PIX_FMT_YVU420: result = "V4L2_PIX_FMT_YVU420"; break;
			case V4L2_PIX_FMT_YUYV: result = "V4L2_PIX_FMT_YUYV"; break;
			case V4L2_PIX_FMT_YYUV: result = "V4L2_PIX_FMT_YYUV"; break;
			case V4L2_PIX_FMT_YVYU: result = "V4L2_PIX_FMT_YVYU"; break;
			case V4L2_PIX_FMT_UYVY: result = "V4L2_PIX_FMT_UYVY"; break;
			case V4L2_PIX_FMT_VYUY: result = "V4L2_PIX_FMT_VYUY"; break;
			case V4L2_PIX_FMT_YUV422P: result = "V4L2_PIX_FMT_YUV422P"; break;
			case V4L2_PIX_FMT_YUV411P: result = "V4L2_PIX_FMT_YUV411P"; break;
			case V4L2_PIX_FMT_Y41P: result = "V4L2_PIX_FMT_Y41P"; break;
			case V4L2_PIX_FMT_YUV444: result = "V4L2_PIX_FMT_YUV444"; break;
			case V4L2_PIX_FMT_YUV555: result = "V4L2_PIX_FMT_YUV555"; break;
			case V4L2_PIX_FMT_YUV565: result = "V4L2_PIX_FMT_YUV565"; break;
			case V4L2_PIX_FMT_YUV32: result = "V4L2_PIX_FMT_YUV32"; break;
			case V4L2_PIX_FMT_YUV410: result = "V4L2_PIX_FMT_YUV410"; break;
			case V4L2_PIX_FMT_YUV420: result = "V4L2_PIX_FMT_YUV420"; break;
			case V4L2_PIX_FMT_HI240: result = "V4L2_PIX_FMT_HI240"; break;
			case V4L2_PIX_FMT_HM12: result = "V4L2_PIX_FMT_HM12"; break;
			case V4L2_PIX_FMT_M420: result = "V4L2_PIX_FMT_M420"; break;

			/* two planes -- one Y, one Cr + Cb interleaved  */
			case V4L2_PIX_FMT_NV12: result = "V4L2_PIX_FMT_NV12"; break;
			case V4L2_PIX_FMT_NV21: result = "V4L2_PIX_FMT_NV21"; break;
			case V4L2_PIX_FMT_NV16: result = "V4L2_PIX_FMT_NV16"; break;
			case V4L2_PIX_FMT_NV61: result = "V4L2_PIX_FMT_NV61"; break;
			case V4L2_PIX_FMT_NV24: result = "V4L2_PIX_FMT_NV24"; break;
			case V4L2_PIX_FMT_NV42: result = "V4L2_PIX_FMT_NV42"; break;

			/* two non contiguous planes - one Y, one Cr + Cb interleaved  */
			case V4L2_PIX_FMT_NV12M: result = "V4L2_PIX_FMT_NV12M"; break;
			case V4L2_PIX_FMT_NV21M: result = "V4L2_PIX_FMT_NV21M"; break;
			case V4L2_PIX_FMT_NV16M: result = "V4L2_PIX_FMT_NV16M"; break;
			case V4L2_PIX_FMT_NV61M: result = "V4L2_PIX_FMT_NV61M"; break;
			case V4L2_PIX_FMT_NV12MT: result = "V4L2_PIX_FMT_NV12MT"; break;
			case V4L2_PIX_FMT_NV12MT_16X16: result = "V4L2_PIX_FMT_NV12MT_16X16"; break;

			/* three non contiguous planes - Y, Cb, Cr */
			case V4L2_PIX_FMT_YUV420M: result = "V4L2_PIX_FMT_YUV420M"; break;
			case V4L2_PIX_FMT_YVU420M: result = "V4L2_PIX_FMT_YVU420M"; break;

			/* Bayer formats - see http://www.siliconimaging.com/RGB%20Bayer.htm */
			case V4L2_PIX_FMT_SBGGR8: result = "V4L2_PIX_FMT_SBGGR8"; break;
			case V4L2_PIX_FMT_SGBRG8: result = "V4L2_PIX_FMT_SGBRG8"; break;
			case V4L2_PIX_FMT_SGRBG8: result = "V4L2_PIX_FMT_SGRBG8"; break;
			case V4L2_PIX_FMT_SRGGB8: result = "V4L2_PIX_FMT_SRGGB8"; break;
			case V4L2_PIX_FMT_SBGGR10: result = "V4L2_PIX_FMT_SBGGR10"; break;
			case V4L2_PIX_FMT_SGBRG10: result = "V4L2_PIX_FMT_SGBRG10"; break;
			case V4L2_PIX_FMT_SGRBG10: result = "V4L2_PIX_FMT_SGRBG10"; break;
			case V4L2_PIX_FMT_SRGGB10: result = "V4L2_PIX_FMT_SRGGB10"; break;
			case V4L2_PIX_FMT_SBGGR12: result = "V4L2_PIX_FMT_SBGGR12"; break;
			case V4L2_PIX_FMT_SGBRG12: result = "V4L2_PIX_FMT_SGBRG12"; break;
			case V4L2_PIX_FMT_SGRBG12: result = "V4L2_PIX_FMT_SGRBG12"; break;
			case V4L2_PIX_FMT_SRGGB12: result = "V4L2_PIX_FMT_SRGGB12"; break;
				/* 10bit raw bayer a-law compressed to 8 bits */
			case V4L2_PIX_FMT_SBGGR10ALAW8: result = "V4L2_PIX_FMT_SBGGR10ALAW8"; break;
			case V4L2_PIX_FMT_SGBRG10ALAW8: result = "V4L2_PIX_FMT_SGBRG10ALAW8"; break;
			case V4L2_PIX_FMT_SGRBG10ALAW8: result = "V4L2_PIX_FMT_SGRBG10ALAW8"; break;
			case V4L2_PIX_FMT_SRGGB10ALAW8: result = "V4L2_PIX_FMT_SRGGB10ALAW8"; break;
				/* 10bit raw bayer DPCM compressed to 8 bits */
			case V4L2_PIX_FMT_SBGGR10DPCM8: result = "V4L2_PIX_FMT_SBGGR10DPCM8"; break;
			case V4L2_PIX_FMT_SGBRG10DPCM8: result = "V4L2_PIX_FMT_SGBRG10DPCM8"; break;
			case V4L2_PIX_FMT_SGRBG10DPCM8: result = "V4L2_PIX_FMT_SGRBG10DPCM8"; break;
			case V4L2_PIX_FMT_SRGGB10DPCM8: result = "V4L2_PIX_FMT_SRGGB10DPCM8"; break;
				
			case V4L2_PIX_FMT_SBGGR16: result = "V4L2_PIX_FMT_SBGGR16"; break;

			/* compressed formats */
			case V4L2_PIX_FMT_MJPEG: result = "V4L2_PIX_FMT_MJPEG"; break;
			case V4L2_PIX_FMT_JPEG: result = "V4L2_PIX_FMT_JPEG"; break;
			case V4L2_PIX_FMT_DV: result = "V4L2_PIX_FMT_DV"; break;
			case V4L2_PIX_FMT_MPEG: result = "V4L2_PIX_FMT_MPEG"; break;
			case V4L2_PIX_FMT_H264: result = "V4L2_PIX_FMT_H264"; break;
			case V4L2_PIX_FMT_H264_NO_SC: result = "V4L2_PIX_FMT_H264_NO_SC"; break;
			case V4L2_PIX_FMT_H264_MVC: result = "V4L2_PIX_FMT_H264_MVC"; break;
			case V4L2_PIX_FMT_H263: result = "V4L2_PIX_FMT_H263"; break;
			case V4L2_PIX_FMT_MPEG1: result = "V4L2_PIX_FMT_MPEG1"; break;
			case V4L2_PIX_FMT_MPEG2: result = "V4L2_PIX_FMT_MPEG2"; break;
			case V4L2_PIX_FMT_MPEG4: result = "V4L2_PIX_FMT_MPEG4"; break;
			case V4L2_PIX_FMT_XVID: result = "V4L2_PIX_FMT_XVID"; break;
			case V4L2_PIX_FMT_VC1_ANNEX_G: result = "V4L2_PIX_FMT_VC1_ANNEX_G"; break;
			case V4L2_PIX_FMT_VC1_ANNEX_L: result = "V4L2_PIX_FMT_VC1_ANNEX_L"; break;
			case V4L2_PIX_FMT_VP8: result = "V4L2_PIX_FMT_VP8"; break;

			/*  Vendor-specific formats   */
			case V4L2_PIX_FMT_CPIA1: result = "V4L2_PIX_FMT_CPIA1"; break;
			case V4L2_PIX_FMT_WNVA: result = "V4L2_PIX_FMT_WNVA"; break;
			case V4L2_PIX_FMT_SN9C10X: result = "V4L2_PIX_FMT_SN9C10X"; break;
			case V4L2_PIX_FMT_SN9C20X_I420: result = "V4L2_PIX_FMT_SN9C20X_I420"; break;
			case V4L2_PIX_FMT_PWC1: result = "V4L2_PIX_FMT_PWC1"; break;
			case V4L2_PIX_FMT_PWC2: result = "V4L2_PIX_FMT_PWC2"; break;
			case V4L2_PIX_FMT_ET61X251: result = "V4L2_PIX_FMT_ET61X251"; break;
			case V4L2_PIX_FMT_SPCA501: result = "V4L2_PIX_FMT_SPCA501"; break;
			case V4L2_PIX_FMT_SPCA505: result = "V4L2_PIX_FMT_SPCA505"; break;
			case V4L2_PIX_FMT_SPCA508: result = "V4L2_PIX_FMT_SPCA508"; break;
			case V4L2_PIX_FMT_SPCA561: result = "V4L2_PIX_FMT_SPCA561"; break;
			case V4L2_PIX_FMT_PAC207: result = "V4L2_PIX_FMT_PAC207"; break;
			case V4L2_PIX_FMT_MR97310A: result = "V4L2_PIX_FMT_MR97310A"; break;
			case V4L2_PIX_FMT_JL2005BCD: result = "V4L2_PIX_FMT_JL2005BCD"; break;
			case V4L2_PIX_FMT_SN9C2028: result = "V4L2_PIX_FMT_SN9C2028"; break;
			case V4L2_PIX_FMT_SQ905C: result = "V4L2_PIX_FMT_SQ905C"; break;
			case V4L2_PIX_FMT_PJPG: result = "V4L2_PIX_FMT_PJPG"; break;
			case V4L2_PIX_FMT_OV511: result = "V4L2_PIX_FMT_OV511"; break;
			case V4L2_PIX_FMT_OV518: result = "V4L2_PIX_FMT_OV511"; break;
			case V4L2_PIX_FMT_STV0680: result = "V4L2_PIX_FMT_STV0680"; break;
			case V4L2_PIX_FMT_TM6000: result = "V4L2_PIX_FMT_TM6000"; break;
			case V4L2_PIX_FMT_CIT_YYVYUY: result = "V4L2_PIX_FMT_CIT_YYVYUY"; break;
			case V4L2_PIX_FMT_KONICA420: result = "V4L2_PIX_FMT_KONICA420"; break;
			case V4L2_PIX_FMT_JPGL: result = "V4L2_PIX_FMT_JPGL"; break;
			case V4L2_PIX_FMT_SE401: result = "V4L2_PIX_FMT_SE401"; break;
			case V4L2_PIX_FMT_S5C_UYVY_JPG: result = "V4L2_PIX_FMT_S5C_UYVY_JPG"; break;

                        /* 10bit and 12bit greyscale packed */
			case V4L2_PIX_FMT_Y10P: result = "V4L2_PIX_FMT_Y10P"; break;
			case V4L2_PIX_FMT_Y12P: result = "V4L2_PIX_FMT_Y12P"; break;

			/* 10bit raw bayer packed, 5 bytes for every 4 pixels */
			case V4L2_PIX_FMT_SBGGR10P: result = "V4L2_PIX_FMT_SBGGR10P"; break;
			case V4L2_PIX_FMT_SGBRG10P: result = "V4L2_PIX_FMT_SGBRG10P"; break;
			case V4L2_PIX_FMT_SGRBG10P: result = "V4L2_PIX_FMT_SGRBG10P"; break;
			case V4L2_PIX_FMT_SRGGB10P: result = "V4L2_PIX_FMT_SRGGB10P"; break;

			/* 12bit raw bayer packed, 6 bytes for every 4 pixels */
			case V4L2_PIX_FMT_SBGGR12P: result = "V4L2_PIX_FMT_SBGGR12P"; break;
			case V4L2_PIX_FMT_SGBRG12P: result = "V4L2_PIX_FMT_SGBRG12P"; break;
			case V4L2_PIX_FMT_SGRBG12P: result = "V4L2_PIX_FMT_SGRBG12P"; break;
			case V4L2_PIX_FMT_SRGGB12P: result = "V4L2_PIX_FMT_SRGGB12P"; break;

			default: result = "<unknown>"; break;
		}
		
		return result;
	}

    
std::string V4l2Helper::ConvertErrno2String(int errnumber)
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

std::string V4l2Helper::ConvertControlID2String(uint32_t controlID)
{
    std::string result = "";
    
    switch(controlID)
    {
	// V4L2_CID_USER_CLASS_BASE
	case V4L2_CID_BRIGHTNESS: 	result = "V4L2_CID_BRIGHTNESS"; break;
	case V4L2_CID_CONTRAST: 	result = "V4L2_CID_CONTRAST"; break;
	case V4L2_CID_SATURATION: 	result = "V4L2_CID_SATURATION"; break;
	case V4L2_CID_HUE: 		result = "V4L2_CID_HUE"; break;
	case V4L2_CID_AUDIO_VOLUME: 	result = "V4L2_CID_AUDIO_VOLUME"; break;
	case V4L2_CID_AUDIO_BALANCE: 	result = "V4L2_CID_AUDIO_BALANCE"; break;
	case V4L2_CID_AUDIO_BASS: 	result = "V4L2_CID_AUDIO_BASS"; break;
	case V4L2_CID_AUDIO_TREBLE: 	result = "V4L2_CID_AUDIO_TREBLE"; break;
	case V4L2_CID_AUDIO_MUTE: 	result = "V4L2_CID_AUDIO_MUTE"; break;
	case V4L2_CID_AUDIO_LOUDNESS: 	result = "V4L2_CID_AUDIO_LOUDNESS"; break;
	case V4L2_CID_BLACK_LEVEL: 	result = "V4L2_CID_BLACK_LEVEL"; break; //deprecated
	case V4L2_CID_AUTO_WHITE_BALANCE: 	result = "V4L2_CID_AUTO_WHITE_BALANCE"; break;
	case V4L2_CID_DO_WHITE_BALANCE: 	result = "V4L2_CID_DO_WHITE_BALANCE"; break;
	case V4L2_CID_RED_BALANCE: 	result = "V4L2_CID_RED_BALANCE"; break;
	case V4L2_CID_BLUE_BALANCE: 	result = "V4L2_CID_BLUE_BALANCE"; break;
	case V4L2_CID_GAMMA: 	result = "V4L2_CID_GAMMA"; break;
	//case V4L2_CID_WHITENESS: 	result = "V4L2_CID_WHITENESS"; break; // deprecated // same as gamma
	case V4L2_CID_EXPOSURE: 	result = "V4L2_CID_EXPOSURE"; break;
	case V4L2_CID_AUTOGAIN: 	result = "V4L2_CID_AUTOGAIN"; break;
	case V4L2_CID_GAIN: 	result = "V4L2_CID_GAIN"; break;
	case V4L2_CID_HFLIP: 	result = "V4L2_CID_HFLIP"; break;
	case V4L2_CID_VFLIP: 	result = "V4L2_CID_VFLIP"; break;
	case V4L2_CID_POWER_LINE_FREQUENCY: 	result = "V4L2_CID_POWER_LINE_FREQUENCY"; break;
	case V4L2_CID_HUE_AUTO: 	result = "V4L2_CID_HUE_AUTO"; break;
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE: 	result = "V4L2_CID_WHITE_BALANCE_TEMPERATURE"; break;
	case V4L2_CID_SHARPNESS: 	result = "V4L2_CID_SHARPNESS"; break;
	case V4L2_CID_BACKLIGHT_COMPENSATION: 	result = "V4L2_CID_BACKLIGHT_COMPENSATION"; break;
	case V4L2_CID_CHROMA_AGC: 	result = "V4L2_CID_CHROMA_AGC"; break;
	case V4L2_CID_COLOR_KILLER: 	result = "V4L2_CID_COLOR_KILLER"; break;
	case V4L2_CID_COLORFX: 	result = "V4L2_CID_COLORFX"; break;
	case V4L2_CID_AUTOBRIGHTNESS: 	result = "V4L2_CID_AUTOBRIGHTNESS"; break;
	case V4L2_CID_BAND_STOP_FILTER: 	result = "V4L2_CID_BAND_STOP_FILTER"; break;
	case V4L2_CID_ROTATE: 	result = "V4L2_CID_ROTATE"; break;
	case V4L2_CID_BG_COLOR: 	result = "V4L2_CID_BG_COLOR"; break;
	case V4L2_CID_CHROMA_GAIN: 	result = "V4L2_CID_CHROMA_GAIN"; break;
	case V4L2_CID_ILLUMINATORS_1: 	result = "V4L2_CID_ILLUMINATORS_1"; break;
	case V4L2_CID_ILLUMINATORS_2: 	result = "V4L2_CID_ILLUMINATORS_2"; break;
	case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE: 	result = "V4L2_CID_MIN_BUFFERS_FOR_CAPTURE"; break;
	case V4L2_CID_MIN_BUFFERS_FOR_OUTPUT: 	result = "V4L2_CID_MIN_BUFFERS_FOR_OUTPUT"; break;
	case V4L2_CID_ALPHA_COMPONENT: 	result = "V4L2_CID_ALPHA_COMPONENT"; break;
	case V4L2_CID_COLORFX_CBCR: 	result = "V4L2_CID_COLORFX_CBCR"; break;
	case V4L2_CID_LASTP1: 	result = "V4L2_CID_LASTP1"; break;

	// V4L2_CID_CAMERA_CLASS_BASE
	case V4L2_CID_EXPOSURE_AUTO: 	result = "V4L2_CID_EXPOSURE_AUTO"; break;
	case V4L2_CID_EXPOSURE_ABSOLUTE: 	result = "V4L2_CID_EXPOSURE_ABSOLUTE"; break;
	case V4L2_CID_EXPOSURE_AUTO_PRIORITY: 	result = "V4L2_CID_EXPOSURE_AUTO_PRIORITY"; break;
	case V4L2_CID_PAN_RELATIVE: 	result = "V4L2_CID_PAN_RELATIVE"; break;
	case V4L2_CID_TILT_RELATIVE: 	result = "V4L2_CID_TILT_RELATIVE"; break;
	case V4L2_CID_PAN_RESET: 	result = "V4L2_CID_PAN_RESET"; break;
	case V4L2_CID_TILT_RESET: 	result = "V4L2_CID_TILT_RESET"; break;
	case V4L2_CID_PAN_ABSOLUTE: 	result = "V4L2_CID_PAN_ABSOLUTE"; break;
	case V4L2_CID_TILT_ABSOLUTE: 	result = "V4L2_CID_TILT_ABSOLUTE"; break;
	case V4L2_CID_FOCUS_ABSOLUTE: 	result = "V4L2_CID_FOCUS_ABSOLUTE"; break;
	case V4L2_CID_FOCUS_RELATIVE: 	result = "V4L2_CID_FOCUS_RELATIVE"; break;
	case V4L2_CID_FOCUS_AUTO: 	result = "V4L2_CID_FOCUS_AUTO"; break;
	case V4L2_CID_ZOOM_ABSOLUTE: 	result = "V4L2_CID_ZOOM_ABSOLUTE"; break;
	case V4L2_CID_ZOOM_RELATIVE: 	result = "V4L2_CID_ZOOM_RELATIVE"; break;
	case V4L2_CID_ZOOM_CONTINUOUS: 	result = "V4L2_CID_ZOOM_CONTINUOUS"; break;
	case V4L2_CID_PRIVACY: 	result = "V4L2_CID_PRIVACY"; break;
	case V4L2_CID_IRIS_ABSOLUTE: 	result = "V4L2_CID_IRIS_ABSOLUTE"; break;
	case V4L2_CID_IRIS_RELATIVE: 	result = "V4L2_CID_IRIS_RELATIVE"; break;
	case V4L2_CID_AUTO_EXPOSURE_BIAS: 	result = "V4L2_CID_AUTO_EXPOSURE_BIAS"; break;
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE: 	result = "V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE"; break;
	case V4L2_CID_WIDE_DYNAMIC_RANGE: 	result = "V4L2_CID_WIDE_DYNAMIC_RANGE"; break;
	case V4L2_CID_IMAGE_STABILIZATION: 	result = "V4L2_CID_IMAGE_STABILIZATION"; break;
	case V4L2_CID_ISO_SENSITIVITY: 	result = "V4L2_CID_ISO_SENSITIVITY"; break;
	case V4L2_CID_ISO_SENSITIVITY_AUTO: 	result = "V4L2_CID_ISO_SENSITIVITY_AUTO"; break;
	case V4L2_CID_EXPOSURE_METERING: 	result = "V4L2_CID_EXPOSURE_METERING"; break;
	case V4L2_CID_SCENE_MODE: 	result = "V4L2_CID_SCENE_MODE"; break;
	case V4L2_CID_3A_LOCK: 	result = "V4L2_CID_3A_LOCK"; break;
	case V4L2_LOCK_EXPOSURE: 	result = "V4L2_LOCK_EXPOSURE"; break;
	case V4L2_LOCK_WHITE_BALANCE: 	result = "V4L2_LOCK_WHITE_BALANCE"; break;
	case V4L2_LOCK_FOCUS: 	result = "V4L2_LOCK_FOCUS"; break;
	case V4L2_CID_AUTO_FOCUS_START: 	result = "V4L2_CID_AUTO_FOCUS_START"; break;
	case V4L2_CID_AUTO_FOCUS_STOP: 	result = "V4L2_CID_AUTO_FOCUS_STOP"; break;
	case V4L2_CID_AUTO_FOCUS_STATUS: 	result = "V4L2_CID_AUTO_FOCUS_STATUS"; break;
	case V4L2_CID_AUTO_FOCUS_RANGE: 	result = "V4L2_CID_AUTO_FOCUS_RANGE"; break;
	default: result = "<unknown>"; break;
    }
    
    return result;
}

}}} /* namespace AVT::Tools::Examples */

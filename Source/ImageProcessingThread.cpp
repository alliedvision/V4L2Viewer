/*=============================================================================
  Copyright (C) 2015 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ImageProcessingThread.cpp

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

#include "ImageProcessingThread.h"
#include "ImageTransf.h"
//#include "VmbImageTransformHelper.hpp"

#include "videodev2_av.h"
#include "Logger.h"

ImageProcessingThread::ImageProcessingThread()
	: m_bAbort(false)
{
}

ImageProcessingThread::~ImageProcessingThread(void)
{
}

int ImageProcessingThread::QueueFrame(v4l2_buffer &buf, uint8_t *&buffer, uint32_t &length, 
									  uint32_t &width, uint32_t &height, uint32_t &pixelformat,
									  uint32_t &payloadSize, uint32_t &bytesPerLine, uint64_t &frameID)
{
	int result = -1;
    
    // make sure Viewer is never working in the past
    // if viewer is too slow we drop the frames.
    if (m_FrameQueue.GetSize() < MAX_QUEUE_SIZE)
    {
		m_FrameQueue.Enqueue(buf, buffer, length, width, height, pixelformat, payloadSize, bytesPerLine, frameID);
		result = 0;
    }
    
    return result;
}

// Set the data for the thread to work with
int ImageProcessingThread::QueueFrame(QImage &image, uint64_t &frameID)
{
    int result = -1;
    
    // make sure Viewer is never working in the past
    // if viewer is too slow we drop the frames.
    if (m_FrameQueue.GetSize() < MAX_QUEUE_SIZE)
    {
		m_FrameQueue.Enqueue(image, frameID);
		result = 0;
    }
    
    return result;
}

// Set the data for the thread to work with
int ImageProcessingThread::QueueFrame(QSharedPointer<MyFrame> pFrame)
{
    int result = -1;
    
    // make sure Viewer is never working in the past
    // if viewer is too slow we drop the frames.
    if (m_FrameQueue.GetSize() < MAX_QUEUE_SIZE)
    {
		m_FrameQueue.Enqueue(pFrame);
		result = 0;
    }
    
    return result;
}

// stop the internal processing thread and wait until the thread is really stopped
void ImageProcessingThread::StartThread()
{
	m_bAbort = false;
  
	start();
	
}

// stop the internal processing thread and wait until the thread is really stopped
void ImageProcessingThread::StopThread()
{
	m_bAbort = true;

	// wait until the thread is stopped
	while (isRunning())
		QThread::msleep(10);
	
	m_FrameQueue.Clear();
}
/*
int ImageProcessingThread::Try2ConvertV4l2Pixelformat2Vimba(uint32_t pixelformat, uint32_t &resPixelformat)
{
    int result = 0;
    
    switch (pixelformat)
    {
        //case V4L2_PIX_FMT_GREY:    resPixelformat = VmbPixelFormatMono8;     break;
        //case V4L2_PIX_FMT_SBGGR8:  resPixelformat = VmbPixelFormatBayerBG8;  break;
        //case V4L2_PIX_FMT_SGBRG8:  resPixelformat = VmbPixelFormatBayerGB8;  break;
        //case V4L2_PIX_FMT_SGRBG8:  resPixelformat = VmbPixelFormatBayerGR8;  break;
        //case V4L2_PIX_FMT_SRGGB8:  resPixelformat = VmbPixelFormatBayerRG8;  break;
        
        //case V4L2_PIX_FMT_GREY10:  resPixelformat = VmbPixelFormatMono10;    break;
        //case V4L2_PIX_FMT_SBGGR10: resPixelformat = VmbPixelFormatBayerGR10; break;
        //case V4L2_PIX_FMT_SGBRG10: resPixelformat = VmbPixelFormatBayerRG10; break;
        //case V4L2_PIX_FMT_SGRBG10: resPixelformat = VmbPixelFormatBayerBG10; break;
        //case V4L2_PIX_FMT_SRGGB10: resPixelformat = VmbPixelFormatBayerGB10; break;
        
        //case V4L2_PIX_FMT_GREY12:  resPixelformat = VmbPixelFormatMono12;    break;
        //case V4L2_PIX_FMT_SBGGR12: resPixelformat = VmbPixelFormatBayerGR12; break;
        //case V4L2_PIX_FMT_SGBRG12: resPixelformat = VmbPixelFormatBayerRG12; break;
        //case V4L2_PIX_FMT_SGRBG12: resPixelformat = VmbPixelFormatBayerBG12; break;
        //case V4L2_PIX_FMT_SRGGB12: resPixelformat = VmbPixelFormatBayerGB12; break;
        
        case V4L2_PIX_FMT_RGB24:   resPixelformat = VmbPixelFormatRgb8;      break;
	case V4L2_PIX_FMT_BGR24:   resPixelformat = VmbPixelFormatBgr8;      break;
        //case V4L2_PIX_FMT_UYVY:    resPixelformat = VmbPixelFormatYuv422;    break;
        //case V4L2_PIX_FMT_RGB565 is done in ImageTransf.cpp
        //case V4L2_PIX_FMT_JPEG is done in ImageTransf.cpp
        default:
            result = -1;
            break;
    }
    
    return result;
}

void  ConvertRAW12ToRAW8(const void *buffer, uint32_t width, uint32_t height, cv::Mat &destMat)
{
    unsigned char *destdata = destMat.data;
    unsigned char *srcdata = (unsigned char *)buffer;
    uint32_t count = 0;
    
    for (int i= 0; i<width*1.5; i++)
    {
       for (int ii= 0; ii<height; ii++)
       {
           if (((count+1)^3) != 0)
                *destdata++ = *srcdata++;
	   count++;
       }
    }
}

void  ConvertRAW10ToRAW8(const void *buffer, uint32_t width, uint32_t height, cv::Mat &destMat)
{
    unsigned char *destdata = destMat.data;
    unsigned char *srcdata = (unsigned char *)buffer;
    uint32_t count = 0;
    
    for (int i= 0; i<width*1.25; i++)
    {
       for (int ii= 0; ii<height; ii++)
       {
           if (((count+1)^5) != 0)
                *destdata++ = *srcdata++;
	   count++;
       }
    }
}

int ImageProcessingThread::Try2ConvertV4l2Pixelformat2OpenCV(uint32_t pixelformat, uint32_t width, uint32_t height, const void *pBuffer, cv::Mat &destMat)
{
    int result = 0;
    
    //emit OnMessage_Signal("converting buffer to RGB888");
	  
    switch (pixelformat)
    {
	case V4L2_PIX_FMT_RGB24: 
	case V4L2_PIX_FMT_BGR24: 
	{
	  cv::Mat frame(cv::Size(width, height), CV_8UC3 , (void*)pBuffer);
	  cv::cvtColor(frame, destMat, CV_BGR2RGB);
	  break; 
	}
	
	case V4L2_PIX_FMT_Y10P:
	{
	  //cv::namedWindow("capture", CV_WINDOW_NORMAL);
	  cv::Mat frame(cv::Size(width, height), CV_16UC1);
	  cv::Mat rgb16bitMat(cv::Size(width, height), CV_16UC1);
	  ConvertRAW10ToRAW8(pBuffer, width, height, frame);
	  cv::cvtColor(frame, rgb16bitMat, CV_BayerRG2RGB);
	  rgb16bitMat.convertTo(destMat, CV_8UC3, 1/255.0);
	  //cv::imshow("capture", destMat);
	  //if (cv::waitKey(10) >= 0)
	  //  break;
	  break;
	}
        case V4L2_PIX_FMT_Y12P:
	{
	  //cv::namedWindow("capture", CV_WINDOW_NORMAL);
	  cv::Mat frame(cv::Size(width, height), CV_16UC1);
	  cv::Mat rgb16bitMat(cv::Size(width, height), CV_16UC1);
	  ConvertRAW12ToRAW8(pBuffer, width, height, frame);
	  cv::cvtColor(frame, rgb16bitMat, CV_BayerRG2RGB);
	  rgb16bitMat.convertTo(destMat, CV_8UC3, 1/255.0);
	  //cv::imshow("capture", destMat);
	  //if (cv::waitKey(10) >= 0)
	  //  break;
	  break;
	}
        case V4L2_PIX_FMT_SBGGR10P: 
	{
	  //cv::namedWindow("capture", CV_WINDOW_NORMAL);
	  cv::Mat frame(cv::Size(width, height), CV_16UC1);
	  cv::Mat rgb16bitMat(cv::Size(width, height), CV_16UC1);
	  ConvertRAW10ToRAW8(pBuffer, width, height, frame);
	  cv::cvtColor(frame, rgb16bitMat, CV_BayerBG2RGB);
	  rgb16bitMat.convertTo(destMat, CV_8UC3, 1/255.0);
	  //cv::imshow("capture", destMat);
	  //if (cv::waitKey(10) >= 0)
	  //  break;
	  break;
	}
        case V4L2_PIX_FMT_SBGGR12P:
	{
	  //cv::namedWindow("capture", CV_WINDOW_NORMAL);
	  cv::Mat frame(cv::Size(width, height), CV_16UC1);
	  cv::Mat rgb16bitMat(cv::Size(width, height), CV_16UC1);
	  ConvertRAW12ToRAW8(pBuffer, width, height, frame);
	  cv::cvtColor(frame, rgb16bitMat, CV_BayerBG2RGB);
	  rgb16bitMat.convertTo(destMat, CV_8UC3, 1/255.0);
	  //cv::imshow("capture", destMat);
	  //if (cv::waitKey(10) >= 0)
	  //  break;
	  break;
	}
        case V4L2_PIX_FMT_SGBRG10P:
	{
	  //cv::namedWindow("capture", CV_WINDOW_NORMAL);
	  cv::Mat frame(cv::Size(width, height), CV_16UC1);
	  cv::Mat rgb16bitMat(cv::Size(width, height), CV_16UC1);
	  ConvertRAW10ToRAW8(pBuffer, width, height, frame);
	  cv::cvtColor(frame, rgb16bitMat, CV_BayerGB2RGB);
	  rgb16bitMat.convertTo(destMat, CV_8UC3, 1/255.0);
	  //cv::imshow("capture", destMat);
	  //if (cv::waitKey(10) >= 0)
	  //  break;
	  break;
	}
        case V4L2_PIX_FMT_SGBRG12P:
	{
	  //cv::namedWindow("capture", CV_WINDOW_NORMAL);
	  cv::Mat frame(cv::Size(width, height), CV_16UC1);
	  cv::Mat rgb16bitMat(cv::Size(width, height), CV_16UC1);
	  ConvertRAW12ToRAW8(pBuffer, width, height, frame);
	  cv::cvtColor(frame, rgb16bitMat, CV_BayerGB2RGB);
	  rgb16bitMat.convertTo(destMat, CV_8UC3, 1/255.0);
	  //cv::imshow("capture", destMat);
	  //if (cv::waitKey(10) >= 0)
	  //  break;
	  break;
	}
        case V4L2_PIX_FMT_SGRBG10P:
	{
	  //cv::namedWindow("capture", CV_WINDOW_NORMAL);
	  cv::Mat frame(cv::Size(width, height), CV_16UC1);
	  cv::Mat rgb16bitMat(cv::Size(width, height), CV_16UC1);
	  ConvertRAW10ToRAW8(pBuffer, width, height, frame);
	  cv::cvtColor(frame, rgb16bitMat, CV_BayerGR2RGB);
	  rgb16bitMat.convertTo(destMat, CV_8UC3, 1/255.0);
	  //cv::imshow("capture", destMat);
	  //if (cv::waitKey(10) >= 0)
	  //  break;
	  break;
	}
        case V4L2_PIX_FMT_SGRBG12P: 
	{
	  //cv::namedWindow("capture", CV_WINDOW_NORMAL);
	  cv::Mat frame(cv::Size(width, height), CV_16UC1);
	  cv::Mat rgb16bitMat(cv::Size(width, height), CV_16UC1);
	  ConvertRAW12ToRAW8(pBuffer, width, height, frame);
	  cv::cvtColor(frame, rgb16bitMat, CV_BayerGR2RGB);
	  rgb16bitMat.convertTo(destMat, CV_8UC3, 1/255.0);
	  //cv::imshow("capture", destMat);
	  //if (cv::waitKey(10) >= 0)
	  //  break;
	  break;
	}
        case V4L2_PIX_FMT_SRGGB10P:
	{
	  //cv::namedWindow("capture", CV_WINDOW_NORMAL);
	  cv::Mat frame(cv::Size(width, height), CV_16UC1);
	  cv::Mat rgb16bitMat(cv::Size(width, height), CV_16UC1);
	  ConvertRAW10ToRAW8(pBuffer, width, height, frame);
	  cv::cvtColor(frame, rgb16bitMat, CV_BayerRG2RGB);
	  rgb16bitMat.convertTo(destMat, CV_8UC3, 1/255.0);
	  //cv::imshow("capture", destMat);
	  //if (cv::waitKey(10) >= 0)
	  //  break;
	  break;
	}
        case V4L2_PIX_FMT_SRGGB12P: 
	{
	  //cv::namedWindow("capture", CV_WINDOW_NORMAL);
	  cv::Mat frame(cv::Size(width, height), CV_16UC1);
	  cv::Mat rgb16bitMat(cv::Size(width, height), CV_16UC1);
	  ConvertRAW12ToRAW8(pBuffer, width, height, frame);
	  cv::cvtColor(frame, rgb16bitMat, CV_BayerRG2RGB);
	  rgb16bitMat.convertTo(destMat, CV_8UC3, 1/255.0);
	  //cv::imshow("capture", destMat);
	  //if (cv::waitKey(10) >= 0)
	  //  break;
	  break;
	}
        
        default:
            result = -1;
            break;
    }
    
    return result;
}
*/
// Do the work within this thread
void ImageProcessingThread::run()
{
	int result = 0;
	
	while (!m_bAbort)
	{
		if(0 < m_FrameQueue.GetSize())
		{
			QSharedPointer<MyFrame> pFrame = m_FrameQueue.Dequeue();
			uint64_t frameID = pFrame->GetFrameId();
			v4l2_buffer buf = pFrame->GetV4l2Image();
			const uint8_t* pBuffer = pFrame->GetBuffer();
			uint32_t length = pFrame->GetBufferlength();
			uint32_t width = pFrame->GetWidth();
			uint32_t height = pFrame->GetHeight();
			uint32_t pixelformat = pFrame->GetPixelformat();
			uint32_t payloadSize = pFrame->GetPayloadSize();
			uint32_t bytesPerLine = pFrame->GetBytesPerLine();
			QImage convertedImage;
            //VmbPixelFormat_t resPixelformat;
	    //cv::Mat destMat(cv::Size(width, height), CV_8UC3); 
	    
	    /*if (0 == Try2ConvertV4l2Pixelformat2OpenCV(pixelformat, width, height, pBuffer, destMat))
	    {
	        convertedImage = QImage((const uchar *) destMat.data, destMat.cols, destMat.rows, destMat.step, QImage::Format_RGB888);
		convertedImage.bits(); //enforce deep copy
	  	result = 0;
	    }
            else if (0 == Try2ConvertV4l2Pixelformat2Vimba(pixelformat, resPixelformat))
            {
                convertedImage = QImage(width, height, QImage::Format_RGB888);
                result = AVT::VmbImageTransform( convertedImage, (void*)pBuffer, width, height, resPixelformat);
            }
	    else*/
            {
                result = AVT::Tools::ImageTransf::ConvertFrame(pBuffer, length, 
                                                               width, height, pixelformat, 
                                                               payloadSize, bytesPerLine, convertedImage);
			}
			
            if (result == 0)
		emit OnFrameReady_Signal(convertedImage, frameID, buf.index);
	    }

	    QThread::msleep(1);
	}
}


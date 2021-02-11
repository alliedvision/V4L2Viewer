/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Helper.cpp

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

#include "Helper.h"

#ifdef _WIN32       /* Windows */
#include <Windows.h>
#else               /* Linux */
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace AVT {
namespace BaseTools {

Helper::Helper (void)
{
}

Helper::~Helper (void)
{
}

void Helper::SwapByteOrder (void *pData, size_t nSize)
{
    unsigned char *pValue = (unsigned char*) pData;

    for (size_t i = 0; i < (nSize / 2); i++)
    {
        unsigned char b = pValue[i];
        pValue[i] = pValue[nSize - 1 - i];
        pValue[nSize - 1 - i] = b;
    }
}


void Helper::OutputDebugWindow(const char *text, ...)
{
}

std::string Helper::VSPrint(const char *text, ...)
{
    std::string output;
    va_list args;
    char *string;
    int sizeText = 0;

    va_start(args, text);
#ifdef _WIN32
    sizeText = _vscprintf(text, args) + 1;
#else
    sizeText = vsnprintf(NULL, 0, text, args) + 1;
#endif
    va_end(args);

#ifdef _WIN32
    string=(char *)malloc(sizeof(char)*sizeText);
#else
    string=(char *)malloc(sizeof(char)*sizeText);
#endif
    if (NULL == string)
       output = "";
    else
    {
        va_start(args, text);
#ifdef _WIN32
        vsprintf_s(string, sizeof(char)*sizeText, text, args);
#else
        vsnprintf(string, sizeof(char)*sizeText, text, args);
#endif
        va_end(args);
        output = string;
        free(string);
    }

    return output;
}

std::string Helper::ConvertPacket2String(uint8_t* buffer, uint32_t length)
{
    std::stringstream output;
    std::stringstream outputText;
    uint32_t i = 0;

    if (NULL != buffer)
    {
        output << "Dump buffer: length=" << length << std::endl << "0x0000 : " << std::setfill('0');
        for (i=0; i<length; i++)
        {
            output << "0x" << std::hex << std::setw(2) << (int)buffer[i] << " ";

            if (buffer[i] >= 0x20 && buffer[i] < 0x7F)
            {
                outputText << (char)buffer[i];
            }
            else
            {
                outputText << ".";
            }

            if (((i+1) % 16) == 0)
            {
                output << outputText.str() << std::endl << "0x" << std::setw(4) << i+1 << " : ";
                outputText.str("");
            }
        }
    }

    if ((i % 16) != 0)
    {
        output << "; " << outputText.str();
    }

    return output.str();
}

void Helper::uSleep(uint32_t msec)
{
#ifdef _WIN32       /* Windows */
    Sleep(msec);
#else               /* Linux */
    usleep(msec * 1000);
#endif
}

std::string Helper::GetTimeStamp()
{
    std::stringstream timestamp;

#ifdef _WIN32       /* Windows */
    SYSTEMTIME st;
    GetLocalTime(&st);

    timestamp << std::setfill('0') << st.wHour << ':' << std::setw(2) << st.wMinute << ':' << std::setw(2) << st.wSecond << '.' << std::setw(3) << st.wMilliseconds;
#else               /* Linux */
    struct timeval tv;
    struct tm *tm;

    gettimeofday(&tv, NULL);
    tm=localtime(&tv.tv_sec);

    timestamp << std::setfill('0') << tm->tm_hour << ':' << std::setw(2) << tm->tm_min << ':' << std::setw(2) << tm->tm_sec << '.' << std::setw(3) << tv.tv_usec;
#endif

    return timestamp.str();
}


} // namespace BaseTools
} // namespace AVT

/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BaseLogger.cpp

  Description: This class represents base logger which shows application
               communicates.

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

#include "LocalMutexLockGuard.h"
#include "Logger.h"

#include <iomanip>
#include <sstream>
#include <unistd.h>

#define MIN_DURATION_BETWEEN_LOG_WRITES_MS             3
#define ALLOWED_DURATION_FOR_LOG_FILE_COMPLETION_MS   10

namespace base {

void BufferThreadProc(BaseLogger *pBaseLogger)
{
    pBaseLogger->BufThreadProc();
}

void DumpThreadProc(BaseLogger *pBaseLogger)
{
    pBaseLogger->DmpThreadProc();
}

void LoggerThreadProc(BaseLogger *pBaseLogger)
{
    pBaseLogger->ThreadProc();
}

BaseLogger::BaseLogger(const std::string &fileName, bool bAppend)
    : m_pFile(NULL)
    , m_bThreadRunning(true)
    , m_bThreadStopped(false)
    , m_bDumpThreadRunning(true)
    , m_bDumpThreadStopped(false)
    , m_bBufferThreadRunning(true)
    , m_bBufferThreadStopped(false)
{
    if(true == bAppend)
    {
        m_pFile = fopen(fileName.c_str(), "at");
    }
    else
    {
        m_pFile = fopen(fileName.c_str(), "wt");
    }

    m_LogTextThread.StartThread((THREAD_START_ROUTINE)LoggerThreadProc, this);
    m_DumpThread.StartThread((THREAD_START_ROUTINE)DumpThreadProc, this);
    m_BufferThread.StartThread((THREAD_START_ROUTINE)BufferThreadProc, this);
}

BaseLogger::~BaseLogger()
{
    m_bBufferThreadRunning = false;
    while(!m_bBufferThreadStopped) usleep(ALLOWED_DURATION_FOR_LOG_FILE_COMPLETION_MS * 1000);

    m_bDumpThreadRunning = false;
    while(!m_bDumpThreadStopped) usleep(ALLOWED_DURATION_FOR_LOG_FILE_COMPLETION_MS * 1000);

    m_bThreadRunning = false;
    while(!m_bThreadStopped) usleep(ALLOWED_DURATION_FOR_LOG_FILE_COMPLETION_MS * 1000);

    if(NULL != m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }

    m_LogTextThread.Join();
    m_DumpThread.Join();
    m_BufferThread.Join();
}

std::string BaseLogger::ConvertTimeStampToString()
{
    std::stringstream timestamp;

    struct timeval tv;
    struct tm *tm;

    gettimeofday(&tv, NULL);
    tm=localtime(&tv.tv_sec);

    timestamp << std::setfill('0') << tm->tm_hour << ':' << std::setw(2) << tm->tm_min << ':' << std::setw(2) << tm->tm_sec << '.' << std::setw(3) << tv.tv_usec;

    return timestamp.str();
}

std::string BaseLogger::ConvertPacketToString(uint8_t* buffer, uint32_t length)
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

void BaseLogger::Log(const std::string &message)
{
    if(m_bThreadRunning)
    {
        std::stringstream text;

        text << ConvertTimeStampToString() << ": " << message;

        if(NULL != m_pFile)
        {
            LocalMutexLockGuard guard(m_Mutex);

            m_OutputQueue.push(text.str());
        }
    }
}

void BaseLogger::LogDump(const std::string &message, uint8_t *buffer, uint32_t length)
{
    if(m_bDumpThreadRunning)
    {
        LocalMutexLockGuard guard(m_DumpMutex);

        Packet* pPacket = new Packet;
        pPacket->buffer.resize(length);
        memcpy(&pPacket->buffer[0], buffer, length);
        pPacket->length = length;
        pPacket->message = message;

        m_DumpQueue.push(pPacket);
    }
}

void BaseLogger::LogBuffer(const std::string &filename, uint8_t *buffer, uint32_t length)
{
    if(m_bDumpThreadRunning)
    {
        LocalMutexLockGuard guard(m_DumpMutex);

        Packet* pPacket = new Packet;
        pPacket->buffer.resize(length);
        memcpy(&pPacket->buffer[0], buffer, length);
        pPacket->length = length;
        pPacket->filename = filename;

        m_BufferQueue.push(pPacket);
    }
}

void BaseLogger::ThreadProc()
{
    PrintStartMessage();

    while(m_bThreadRunning)
    {
        if(NULL != m_pFile && m_OutputQueue.size() > 0)
        {
            std::string text;
            {
                LocalMutexLockGuard guard(m_Mutex);

                text = m_OutputQueue.front();
                m_OutputQueue.pop();
            }
            fprintf(m_pFile, "%s\n", text.c_str());
            fflush(m_pFile);
        }

        usleep(MIN_DURATION_BETWEEN_LOG_WRITES_MS * 1000);
    }

    PrintExitMessage();

    m_bThreadStopped = true;
}

void BaseLogger::DmpThreadProc()
{
    while(m_bDumpThreadRunning)
    {
        if(m_DumpQueue.size() > 0)
        {
            std::string text;
            Packet* pPacket;
            {
                LocalMutexLockGuard guard(m_DumpMutex);

                pPacket = m_DumpQueue.front();
                m_DumpQueue.pop();
            }
            text = ConvertPacketToString(&pPacket->buffer[0], pPacket->length);
            Log(pPacket->message + text);
            delete pPacket;
        }

        usleep(MIN_DURATION_BETWEEN_LOG_WRITES_MS * 1000);
    }

    PrintDumpExitMessage();

    m_bDumpThreadStopped = true;
}

void BaseLogger::BufThreadProc()
{
    while(m_bBufferThreadRunning)
    {
        if(m_BufferQueue.size() > 0)
        {
            Packet* pPacket;

            {
                LocalMutexLockGuard guard(m_BufferMutex);
                pPacket = m_BufferQueue.front();
                m_BufferQueue.pop();
            }

            FILE *localFile = fopen(pPacket->filename.c_str(), "wb");

            if(NULL != localFile)
            {
                fwrite(&pPacket->buffer[0], 1, pPacket->length, localFile);

                fflush(localFile);
                fclose(localFile);
            }
            delete pPacket;
        }

        usleep(MIN_DURATION_BETWEEN_LOG_WRITES_MS * 1000);
    }

    PrintBufferExitMessage();

    m_bBufferThreadStopped = true;
}

void BaseLogger::PrintStartMessage()
{
    std::stringstream text;

    text << ConvertTimeStampToString() << ": +++++ logger started. +++++";
    if(NULL != m_pFile)
    {
        fprintf(m_pFile, "%s\n", text.str().c_str());
        fflush(m_pFile);
    }
}

void BaseLogger::PrintExitMessage()
{
    std::stringstream text;

    if(NULL == m_pFile)
    {
        return;
    }

    text << ConvertTimeStampToString() << ": ***** logger exiting, " << m_OutputQueue.size() << " messages left";
    fprintf(m_pFile, "%s\n", text.str().c_str());
    fflush(m_pFile);

    while(m_OutputQueue.size() > 0)
    {
        std::string out = m_OutputQueue.front();
        m_OutputQueue.pop();
        fprintf(m_pFile, "%s\n", out.c_str());
        fflush(m_pFile);
    }

    text.str("");
    text << ConvertTimeStampToString() << ": ***** logger stopped. *****";
    fprintf(m_pFile, "%s\n", text.str().c_str());
    fflush(m_pFile);
}

void BaseLogger::PrintDumpExitMessage()
{
    std::stringstream text;

    if(NULL == m_pFile)
    {
        return;
    }

    text << ConvertTimeStampToString() << ": ***** Dump logger exiting, " << m_DumpQueue.size() << " dumps left";
    fprintf(m_pFile, "%s\n", text.str().c_str());
    fflush(m_pFile);

    while(m_DumpQueue.size() > 0)
    {
        std::string out;
        Packet* pPacket = m_DumpQueue.front();
        m_DumpQueue.pop();
        out = ConvertPacketToString(&pPacket->buffer[0], pPacket->length);
        delete pPacket;

        fprintf(m_pFile, "%s\n", out.c_str());
        fflush(m_pFile);
    }

    text.str("");
    text << ConvertTimeStampToString() << ": ***** Dump logger stopped. *****";
    fprintf(m_pFile, "%s\n", text.str().c_str());
    fflush(m_pFile);
}

void BaseLogger::PrintBufferExitMessage()
{
    while(m_BufferQueue.size() > 0)
    {
        Packet* pPacket = m_BufferQueue.front();
        FILE *localFile = fopen(pPacket->filename.c_str(), "wb");

        m_BufferQueue.pop();

        if(NULL != localFile)
        {
            fwrite(&pPacket->buffer[0], 1, pPacket->length, localFile);

            fflush(localFile);
            fclose(localFile);
        }
        delete pPacket;
    }
}

} // namespace base

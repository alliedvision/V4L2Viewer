/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BaseLogger.cpp

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

#include "Logger.h"
#include "Helper.h"
#include <sstream>
#include <iomanip>

namespace AVT {
namespace BaseTools {

void BufferThreadProc(Logger *threadParams)
{
    threadParams->BufThreadProc();
}

void DumpThreadProc(Logger *threadParams)
{
    threadParams->DmpThreadProc();
}

void LoggerThreadProc(Logger *threadParams)
{
    threadParams->ThreadProc();
}


Logger::Logger(const std::string &fileName, bool bAppend)
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

    m_pLogTextThread.StartThread((LPTHREAD_START_ROUTINE)LoggerThreadProc, this);
    m_pDumpThread.StartThread((LPTHREAD_START_ROUTINE)DumpThreadProc, this);
    m_pBufferThread.StartThread((LPTHREAD_START_ROUTINE)BufferThreadProc, this);
}

Logger::~Logger()
{
    m_bBufferThreadRunning = false;
    while(!m_bBufferThreadStopped) Helper::uSleep(10);

    m_bDumpThreadRunning = false;
    while(!m_bDumpThreadStopped) Helper::uSleep(10);

    m_bThreadRunning = false;
    while(!m_bThreadStopped) Helper::uSleep(10);

    if(NULL != m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }

    m_pLogTextThread.Join();
    m_pDumpThread.Join();
    m_pBufferThread.Join();
}

void Logger::Log(const std::string &rStrMessage)
{
    if(m_bThreadRunning)
    {
        std::stringstream text;

        text << Helper::GetTimeStamp() << ": " << rStrMessage;

        if(NULL != m_pFile)
        {
            AutoLocalMutex guard(m_Mutex);

            m_OutputQueue.push(text.str());
        }
    }
}

void Logger::LogDump(const std::string &rStrMessage, uint8_t *buffer, uint32_t length)
{
    if(m_bDumpThreadRunning)
    {
        AutoLocalMutex guard(m_DumpMutex);

        PPACKET pack = new PACKET;
        pack->Buffer.resize(length);
        memcpy(&pack->Buffer[0], buffer, length);
        pack->Length = length;
        pack->Message = rStrMessage;

        m_DumpQueue.push(pack);
    }
}

void Logger::LogBuffer(const std::string &rFileName, uint8_t *buffer, uint32_t length)
{
    if(m_bDumpThreadRunning)
    {
        AutoLocalMutex guard(m_DumpMutex);

        PPACKET pack = new PACKET;
        pack->Buffer.resize(length);
        memcpy(&pack->Buffer[0], buffer, length);
        pack->Length = length;
        pack->FileName = rFileName;

        m_BufferQueue.push(pack);
    }
}

void Logger::ThreadProc()
{
    PrintStartMessage();

    while(m_bThreadRunning)
    {
        if(NULL != m_pFile && m_OutputQueue.size() > 0)
        {
            std::string text;
            {
                AutoLocalMutex guard(m_Mutex);

                text = m_OutputQueue.front();
                m_OutputQueue.pop();
            }
            fprintf(m_pFile, "%s\n", text.c_str());
            fflush(m_pFile);
        }

        Helper::uSleep(3);
    }

    PrintExitMessage();

    m_bThreadStopped = true;
}

void Logger::DmpThreadProc()
{
    while(m_bDumpThreadRunning)
    {
        if(m_DumpQueue.size() > 0)
        {
            std::string text;
            PPACKET pack;
            {
                AutoLocalMutex guard(m_DumpMutex);

                pack = m_DumpQueue.front();
                m_DumpQueue.pop();
            }
            text = Helper::ConvertPacket2String(&pack->Buffer[0], pack->Length);
            Log(pack->Message + text);
            delete pack;
        }

        Helper::uSleep(3);
    }

    PrintDumpExitMessage();

    m_bDumpThreadStopped = true;
}

void Logger::BufThreadProc()
{
    while(m_bBufferThreadRunning)
    {
        if(m_BufferQueue.size() > 0)
        {
            PPACKET pack;

            {
                AutoLocalMutex guard(m_BufferMutex);
                pack = m_BufferQueue.front();
                m_BufferQueue.pop();
            }

            FILE *localFile = fopen(pack->FileName.c_str(), "wb");

            if(NULL != localFile)
            {
                fwrite(&pack->Buffer[0], 1, pack->Length, localFile);

                fflush(localFile);
                fclose(localFile);
            }
            delete pack;
        }

        Helper::uSleep(3);
    }

    PrintBufferExitMessage();

    m_bBufferThreadStopped = true;
}

void Logger::PrintStartMessage()
{
    std::stringstream text;

    text << Helper::GetTimeStamp() << ": +++++ logger started. +++++";
    if(NULL != m_pFile)
    {
        fprintf(m_pFile, "%s\n", text.str().c_str());
        fflush(m_pFile);
    }
}

void Logger::PrintExitMessage()
{
    std::stringstream text;

    if(NULL == m_pFile)
    {
        return;
    }

    text << Helper::GetTimeStamp() << ": ***** logger exiting, " << m_OutputQueue.size() << " messages left";
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
    text << Helper::GetTimeStamp() << ": ***** logger stopped. *****";
    fprintf(m_pFile, "%s\n", text.str().c_str());
    fflush(m_pFile);
}

void Logger::PrintDumpExitMessage()
{
    std::stringstream text;

    if(NULL == m_pFile)
    {
        return;
    }

    text << Helper::GetTimeStamp() << ": ***** Dump logger exiting, " << m_DumpQueue.size() << " dumps left";
    fprintf(m_pFile, "%s\n", text.str().c_str());
    fflush(m_pFile);

    while(m_DumpQueue.size() > 0)
    {
        std::string out;
        PPACKET pack = m_DumpQueue.front();
        m_DumpQueue.pop();
        out = Helper::ConvertPacket2String(&pack->Buffer[0], pack->Length);
        delete pack;

        fprintf(m_pFile, "%s\n", out.c_str());
        fflush(m_pFile);
    }

    text.str("");
    text << Helper::GetTimeStamp() << ": ***** Dump logger stopped. *****";
    fprintf(m_pFile, "%s\n", text.str().c_str());
    fflush(m_pFile);
}

void Logger::PrintBufferExitMessage()
{
    while(m_BufferQueue.size() > 0)
    {
        PPACKET pack = m_BufferQueue.front();
        FILE *localFile = fopen(pack->FileName.c_str(), "wb");

        m_BufferQueue.pop();

        if(NULL != localFile)
        {
            fwrite(&pack->Buffer[0], 1, pack->Length, localFile);

            fflush(localFile);
            fclose(localFile);
        }
        delete pack;
    }
}

} // namespace BaseTools
} // namespace AVT

#include "Logger.h"
#include "Helper.h"
#include <sstream>
#include <iomanip>

namespace AVT {
namespace BaseTools {

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
    }

    Logger::~Logger()
    {
        m_bDumpThreadRunning = false;
        while (!m_bDumpThreadStopped) Helper::uSleep(10);
    
        m_bThreadRunning = false;
        while (!m_bThreadStopped) Helper::uSleep(10);
    
        //SVWTools::Sleep(100);

        if(NULL != m_pFile)
        {
            fclose(m_pFile);
            m_pFile = NULL;
        }

        m_pLogTextThread.Join();
        m_pDumpThread.Join();
    }

    void Logger::Log(const std::string &rStrMessage)
    {
        if (m_bThreadRunning)
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
        if (m_bDumpThreadRunning)
        {
            AutoLocalMutex guard(m_DumpMutex);
        
            PPACKET pack = new PACKET;
            pack->Buffer.resize(length);
            memcpy (&pack->Buffer[0], buffer, length);
            pack->Length = length;
            pack->Message = rStrMessage;

            m_DumpQueue.push(pack);
        }
    }

    void Logger::ThreadProc()
    {
        PrintStartMessage();

        while (m_bThreadRunning)
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
        while (m_bDumpThreadRunning)
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

    void Logger::PrintStartMessage()
    {
        std::stringstream text;

        text << Helper::GetTimeStamp() << ": +++++ logger started. +++++";
        if (NULL != m_pFile)
        {
            fprintf(m_pFile, "%s\n", text.str().c_str());
            fflush(m_pFile);
        }
    }

    void Logger::PrintExitMessage()
    {
        std::stringstream text;
    
        if (NULL == m_pFile)
        {
            return;
        }

        text << Helper::GetTimeStamp() << ": ***** logger exiting, " << m_OutputQueue.size() << " messages left";
        fprintf(m_pFile, "%s\n", text.str().c_str());
        fflush(m_pFile);

        while (m_OutputQueue.size() > 0)
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
    
        if (NULL == m_pFile)
        {
            return;
        }

        text << Helper::GetTimeStamp() << ": ***** Dump logger exiting, " << m_DumpQueue.size() << " dumps left";
        fprintf(m_pFile, "%s\n", text.str().c_str());
        fflush(m_pFile);

        while (m_DumpQueue.size() > 0)
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

}} /* namespace AVT::BaseTools */

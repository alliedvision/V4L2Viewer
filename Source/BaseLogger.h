#ifndef PCIETOOLS_LOGGER_INCLUDE
#define PCIETOOLS_LOGGER_INCLUDE

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <queue>
#include <vector>
#include "Thread.h"
#include "LoggerMutex.h"

namespace AVT {
namespace BaseTools {

    typedef struct _PACKET
    {
        std::string            Message;
        std::vector<uint8_t>   Buffer;
        uint32_t               Length;
    } PACKET, *PPACKET;

    class Logger
    {
    public:
        Logger(const std::string &fileName, bool bAppend = false);
        virtual ~Logger();

        void ThreadProc();
        void DmpThreadProc();

        void Log(const std::string &rStrMessage);
        void LogDump(const std::string &rStrMessage, uint8_t *buffer, uint32_t length);
    
    private:
        void PrintStartMessage();
        void PrintExitMessage();
        void PrintDumpExitMessage();

        FILE                                    *m_pFile;
        LocalMutex                              m_Mutex;
        LocalMutex                              m_DumpMutex;
        Thread                                  m_pLogTextThread;
        Thread                                  m_pDumpThread;
        std::queue<std::string>                 m_OutputQueue;
        std::queue<PPACKET>                     m_DumpQueue;
        bool                                    m_bThreadRunning;
        bool                                    m_bThreadStopped;
        bool                                    m_bDumpThreadRunning;
        bool                                    m_bDumpThreadStopped;

    };

}} /* namespace AVT::BaseTools */

#endif /* PCIETOOLS_LOGGER_INCLUDE */


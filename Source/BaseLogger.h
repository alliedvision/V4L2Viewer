#ifndef PCIETOOLS_LOGGER_INCLUDE
#define PCIETOOLS_LOGGER_INCLUDE

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <queue>
#include <vector>
#include <string>
#include "Thread.h"
#include "LoggerMutex.h"

namespace AVT {
namespace BaseTools {

    typedef struct _PACKET
    {
        std::string            Message;
        std::vector<uint8_t>   Buffer;
        uint32_t               Length;
	std::string	       FileName;
    } PACKET, *PPACKET;

    class Logger
    {
    public:
        Logger(const std::string &fileName, bool bAppend = false);
        virtual ~Logger();

        void ThreadProc();
        void DmpThreadProc();
	void BufThreadProc();

        void Log(const std::string &rStrMessage);
        void LogDump(const std::string &rStrMessage, uint8_t *buffer, uint32_t length);
	void LogBuffer(const std::string &rFileName, uint8_t *buffer, uint32_t length);
    
    private:
        void PrintStartMessage();
        void PrintExitMessage();
        void PrintDumpExitMessage();
	void PrintBufferExitMessage();

        FILE                                    *m_pFile;
        LocalMutex                              m_Mutex;
        LocalMutex                              m_DumpMutex;
        LocalMutex                              m_BufferMutex;
        Thread                                  m_pLogTextThread;
        Thread                                  m_pDumpThread;
        Thread                                  m_pBufferThread;
        std::queue<std::string>                 m_OutputQueue;
        std::queue<PPACKET>                     m_DumpQueue;
        std::queue<PPACKET>                     m_BufferQueue;
        bool                                    m_bThreadRunning;
        bool                                    m_bThreadStopped;
        bool                                    m_bDumpThreadRunning;
        bool                                    m_bDumpThreadStopped;
	bool                                    m_bBufferThreadRunning;
        bool                                    m_bBufferThreadStopped;
    };

}} /* namespace AVT::BaseTools */

#endif /* PCIETOOLS_LOGGER_INCLUDE */


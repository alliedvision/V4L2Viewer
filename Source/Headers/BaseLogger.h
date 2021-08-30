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


#ifndef BASELOGGER_H
#define BASELOGGER_H

#include "LocalMutex.h"
#include "Thread.h"

#include <queue>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

namespace base {

struct Packet
{
    std::string message;
    std::vector<uint8_t> buffer;
    uint32_t length;
    std::string filename;
};

class BaseLogger
{
public:
    BaseLogger(const std::string &fileName, bool bAppend = false);
    virtual ~BaseLogger();

    // This function starts processing in the thread
    void ThreadProc();
    // This function dumps processing in the thread
    void DmpThreadProc();
    // This function saves log buffer to the file
    void BufThreadProc();

    // This function adds new packets to the output queue
    //
    // Parameters:
    // [in] (const std::string &) message - new message to log
    void Log(const std::string &message);
    // Ths function add new packet to the dump queue
    //
    // Parameters:
    // [in] (const std::string &) message
    // [in] (uint8_t *) buffer
    // [in] (uint32_t) length
    void LogDump(const std::string &message, uint8_t *buffer, uint32_t length);
    // This function adds new packet to the queue buffer
    //
    // Parameters:
    // [in] (const std::string &) fileName - name of the output file
    // [in] (uint8_t *) buffer
    // [in] (uint32_t) length
    void LogBuffer(const std::string &fileName, uint8_t *buffer, uint32_t length);

private:
    // This function prints start message
    void PrintStartMessage();
    // This function prints exit message
    void PrintExitMessage();
    // This function prints dump exit message
    void PrintDumpExitMessage();
    // This function prints buffer exit message
    void PrintBufferExitMessage();

private:
    // This function converts timestamp to string
    //
    // Returns:
    // (std::string) - timestamp
    static
    std::string ConvertTimeStampToString();
    // This function converts packet to string
    //
    // Parameters:
    // [in] (uint8_t *) buffer
    // [in] (uint32_t) length
    //
    // Returns:
    // (std::string) - packet
    static
    std::string ConvertPacketToString (uint8_t *buffer, uint32_t length);

    FILE                          *m_pFile;
    LocalMutex                    m_Mutex;
    LocalMutex                    m_DumpMutex;
    LocalMutex                    m_BufferMutex;
    Thread                        m_LogTextThread;
    Thread                        m_DumpThread;
    Thread                        m_BufferThread;
    std::queue<std::string>       m_OutputQueue;
    std::queue<Packet*>           m_DumpQueue;
    std::queue<Packet*>           m_BufferQueue;
    bool                          m_bThreadRunning;
    bool                          m_bThreadStopped;
    bool                          m_bDumpThreadRunning;
    bool                          m_bDumpThreadStopped;
    bool                          m_bBufferThreadRunning;
    bool                          m_bBufferThreadStopped;
};

} // namespace base

#endif // BASELOGGER_H

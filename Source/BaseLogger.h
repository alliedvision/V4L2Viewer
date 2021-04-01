/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BaseLogger.h

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

    void ThreadProc();
    void DmpThreadProc();
    void BufThreadProc();

    void Log(const std::string &message);
    void LogDump(const std::string &message, uint8_t *buffer, uint32_t length);
    void LogBuffer(const std::string &fileName, uint8_t *buffer, uint32_t length);

private:
    void PrintStartMessage();
    void PrintExitMessage();
    void PrintDumpExitMessage();
    void PrintBufferExitMessage();

private:
    static
    std::string ConvertTimeStampToString();

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

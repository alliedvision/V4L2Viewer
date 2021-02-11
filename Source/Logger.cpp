/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        Logger.cpp

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

#include "Helper.h"

#ifdef _WIN32
#include "windows.h"
#endif
#include "Logger.h"

QSharedPointer<AVT::BaseTools::Logger> Logger::m_pLogger;
bool Logger::m_LogSwitch = false;

Logger::Logger(void)
{
}


Logger::~Logger(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////
// PCIe Logging
//////////////////////////////////////////////////////////////////////////////////////

void Logger::SetV4L2Logger(const std::string &rLogFileName)
{
    if (NULL == m_pLogger)
    {
        m_pLogger = QSharedPointer<AVT::BaseTools::Logger>(new AVT::BaseTools::Logger(rLogFileName));
        m_LogSwitch = true;
    }
}

void Logger::Log(const std::string &rMessage)
{
    if (m_LogSwitch)
        m_pLogger->Log(rMessage);
}

void Logger::LogEx(const char *text, ...)
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
    string = (char *)malloc(sizeof(char)*sizeText);
#else
    string = (char *)malloc(sizeof(char)*sizeText);
#endif

    if (NULL == string)
        output = "<conversion failed>";
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

    if (m_pLogger == NULL)
    {
        m_LogSwitch = true;
        Logger::SetV4L2Logger("Noname.log");
    }

    if (m_LogSwitch)
        m_pLogger->Log(output);
}

void Logger::LogDump(const std::string &rMessage, uint8_t *buffer, uint32_t length)
{
    if (m_LogSwitch)
        m_pLogger->LogDump(rMessage, buffer, length);
}

void Logger::LogBuffer(const std::string &rFileName, uint8_t *buffer, uint32_t length)
{
    m_pLogger->LogBuffer(rFileName, buffer, length);
}

void Logger::LogSwitch(bool flag)
{
    LogEx("Logger switched %s", (flag)?"on":"off");

    m_LogSwitch = flag;
}


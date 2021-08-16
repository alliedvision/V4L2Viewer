/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        Logger.cpp

Description: This class provides operations for the logger, and calls
             BaseLogger functions.

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

QSharedPointer<base::BaseLogger> Logger::m_pBaseLogger;
bool Logger::m_LogSwitch = false;

Logger::Logger(void)
{
}

Logger::~Logger(void)
{
}

void Logger::InitializeLogger(const std::string &logFilename)
{
    if (NULL == m_pBaseLogger)
    {
        m_pBaseLogger = QSharedPointer<base::BaseLogger>(new base::BaseLogger(logFilename));
        m_LogSwitch = true;
    }
}

void Logger::Log(const std::string &message)
{
    if (m_LogSwitch)
        m_pBaseLogger->Log(message);
}

void Logger::LogEx(const char *text, ...)
{
    std::string output;
    va_list args;
    char *string;
    int sizeText = 0;

    va_start(args, text);
    sizeText = vsnprintf(NULL, 0, text, args) + 1;
    va_end(args);

    string = (char *)malloc(sizeof(char)*sizeText);

    if (NULL == string)
        output = "<conversion failed>";
    else
    {
        va_start(args, text);
        vsnprintf(string, sizeof(char)*sizeText, text, args);
        va_end(args);
        output = string;
        free(string);
    }

    if (m_pBaseLogger == NULL)
    {
        m_LogSwitch = true;
        Logger::InitializeLogger("Noname.log");
    }

    if (m_LogSwitch)
        m_pBaseLogger->Log(output);
}

void Logger::LogDump(const std::string &message, uint8_t *buffer, uint32_t length)
{
    if (m_LogSwitch)
        m_pBaseLogger->LogDump(message, buffer, length);
}

void Logger::LogBuffer(const std::string &filename, uint8_t *buffer, uint32_t length)
{
    m_pBaseLogger->LogBuffer(filename, buffer, length);
}

void Logger::LogSwitch(bool flag)
{
    if (flag)
    {
        m_LogSwitch = true;
        LogEx("Logger switched on");
    }
    else
    {
        LogEx("Logger switched off");
        m_LogSwitch = false;
    }
}


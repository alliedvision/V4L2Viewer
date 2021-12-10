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


#include "Logger.h"

#include <sstream>

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

void Logger::LogEx(const char *filename, int line, const char *text, ...)
{
    std::stringstream prefixStream;
    if(line > -1)
    {
        prefixStream << filename << ":" << line << ": ";
    }

    std::string output = prefixStream.str();
    va_list args;
    char *string;
    int sizeText = 0;

    va_start(args, text);
    sizeText = vsnprintf(NULL, 0, text, args) + 1;
    va_end(args);

    string = (char *)malloc(sizeof(char)*sizeText);

    if (NULL == string)
        output += "<conversion failed>";
    else
    {
        va_start(args, text);
        vsnprintf(string, sizeof(char)*sizeText, text, args);
        va_end(args);
        output += string;
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
        LogEx("", -1, "Logger switched on");
    }
    else
    {
        LogEx("", -1, "Logger switched off");
        m_LogSwitch = false;
    }
}


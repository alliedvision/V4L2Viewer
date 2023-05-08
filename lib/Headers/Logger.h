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


#ifndef LOGGER_H
#define LOGGER_H

#include "BaseLogger.h"

#include <QSharedPointer>

#include <string>

class Logger
{
public:
    Logger(void);
    ~Logger(void);

    // This function initializes logger
    //
    // Parameters:
    // [in] (const std::string &) logFilename
    static void InitializeLogger(const std::string &logFilename);
    // This function logs passed message
    //
    // Parameters:
    // [in] (const std::string &) message
    static void Log(const std::string &message);
    // This function logs passed message with additional arguments.
    // It is preferable to use the LOG_EX macro below
    //
    // Parameters:
    // [in] (const char *) filename
    // [in] (int) line - set to a negative number to disable logging of filename and line number
    // [in] (const char *text) text
    static void LogEx(const char *filename, int line, const char *text, ...);
    // This function dumps passed message
    //
    // Parameters:
    // [in] (const std::string &) message
    // [in] (uint8_t *) buffer
    // [in] (uint32_t) length
    static void LogDump(const std::string &message, uint8_t *buffer, uint32_t length);
    // This function logs buffer
    //
    // Parameters:
    // [in] (const std::string &) filename
    // [in] (uint8_t *) buffer
    // [in] (uint32_t) length
    static void LogBuffer(const std::string &filename, uint8_t *buffer, uint32_t length);
    // This function swtich logger's turn state
    //
    // Parameters:
    // [in] (bool) flag
    static void LogSwitch(bool flag);

private:
    static QSharedPointer<base::BaseLogger> m_pBaseLogger;
    static bool m_LogSwitch;
};

constexpr const char* filepathToFilename(const char* filepath)
{
    const char* begin = filepath;
    for(const char* p = filepath; *p != '\0'; ++p)
    {
        if(*p == '/')
        {
            begin = p + 1;
        }
    }
    return begin;
}

#define __FILENAME__ filepathToFilename(__FILE__)

#define LOG_EX(...) \
    Logger::LogEx(__FILENAME__, __LINE__, __VA_ARGS__)

#endif // LOGGER_H

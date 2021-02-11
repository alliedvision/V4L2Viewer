/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        Logger.h

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

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include "BaseLogger.h"

#include <QSharedPointer>

class Logger
{
public:
    Logger(void);
    ~Logger(void);

    static void SetV4L2Logger(const std::string &rLogFileName);
    static void Log(const std::string &rMessage);
    static void LogEx(const char *text, ...);
    static void LogDump(const std::string &rMessage, uint8_t *buffer, uint32_t length);
    static void LogBuffer(const std::string &rFileName, uint8_t *buffer, uint32_t length);
    static void LogSwitch(bool flag);

private:
    static QSharedPointer<AVT::BaseTools::Logger> m_pLogger;
    static bool m_LogSwitch;

};

#endif // LOGGER_H

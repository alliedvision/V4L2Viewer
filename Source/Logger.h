#ifndef Logger_H
#define Logger_H

#include <string>
#include "BaseLogger.h"

#include <QSharedPointer>

class Logger
{
public:
    Logger(void);
    ~Logger(void);

    static void SetPCIeLogger(const std::string &rLogFileName);
    static void Log(const std::string &rMessage);
    static void LogEx(const char *text, ...);
    static void LogDump(const std::string &rMessage, uint8_t *buffer, uint32_t length);
    static void LogBuffer(const std::string &rFileName, uint8_t *buffer, uint32_t length);
    static void LogSwitch(bool flag);

private:
    static QSharedPointer<AVT::BaseTools::Logger> m_pLogger;
    static bool m_LogSwitch;

};

#endif /* Logger_H */

#include "Helper.h"

#ifdef _WIN32
#include "windows.h"
#endif
#include "Logger.h"

QSharedPointer<AVT::BaseTools::Logger> Logger::m_pLogger;

Logger::Logger(void)
{
}


Logger::~Logger(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////
// PCIe Logging
//////////////////////////////////////////////////////////////////////////////////////
    
void Logger::SetPCIeLogger(const std::string &rLogFileName)
{
    if (NULL == m_pLogger)
    {
        m_pLogger = QSharedPointer<AVT::BaseTools::Logger>(new AVT::BaseTools::Logger(rLogFileName));
    }
}

void Logger::Log(const std::string &rMessage)
{
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
        Logger::SetPCIeLogger("LibCSITestLog.log");

    m_pLogger->Log(output);
}
    
void Logger::LogDump(const std::string &rMessage, uint8_t *buffer, uint32_t length)
{
    m_pLogger->LogDump(rMessage, buffer, length);
}


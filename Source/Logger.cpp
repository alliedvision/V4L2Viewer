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
    
void Logger::SetPCIeLogger(const std::string &rLogFileName)
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
        Logger::SetPCIeLogger("Noname.log");
    }

    if (m_LogSwitch)
	m_pLogger->Log(output);
}
    
void Logger::LogDump(const std::string &rMessage, uint8_t *buffer, uint32_t length)
{
    if (m_LogSwitch)
	m_pLogger->LogDump(rMessage, buffer, length);
}

void Logger::LogSwitch(bool flag)
{
    LogEx("Logger switched %s", (flag)?"on":"off");
    
    m_LogSwitch = flag;
}


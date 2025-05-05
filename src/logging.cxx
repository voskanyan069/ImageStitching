#include "logging.hxx"

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

std::string C_RED = "\033[31m";
std::string C_YEL = "\033[33m";
std::string C_CYN = "\033[36m";
std::string C_RST = "\033[0m";

bool Logging::m_bDisabled = false;
std::ostream* Logging::m_os = &std::cout;

void Logging::Log(LogType type, const char* format, ...)
{
    if ( m_bDisabled )
    {
        return;
    }
    va_list args;
    va_start(args, format);
    va_end(args);
    switch (type)
    {
    case LogType::INFO:
        LogInfo(format, args);
        break;
    case LogType::WARN:
        LogWarn(format, args);
        break;
    case LogType::ERROR:
        LogError(format, args);
        break;
    }
}

void Logging::LogInfo(const char* format, ...)
{
    if ( m_bDisabled )
    {
        return;
    }
    *m_os << C_CYN << " [INFO] > ";
    va_list args;
    va_start(args, format);
    std::vector<char> buffer(1024);
    vsnprintf(buffer.data(), buffer.size(), format, args);
    va_end(args);
    *m_os << buffer.data() << C_RST << std::endl;
}

void Logging::LogWarn(const char* format, ...)
{
    if ( m_bDisabled )
    {
        return;
    }
    *m_os << C_YEL << " [WARNING] > ";
    va_list args;
    va_start(args, format);
    std::vector<char> buffer(1024);
    vsnprintf(buffer.data(), buffer.size(), format, args);
    va_end(args);
    *m_os << buffer.data() << C_RST << std::endl;
}

void Logging::LogError(const char* format, ...)
{
    if ( m_bDisabled )
    {
        return;
    }
    *m_os << C_RED << " [ERROR] > ";
    va_list args;
    va_start(args, format);
    std::vector<char> buffer(1024);
    vsnprintf(buffer.data(), buffer.size(), format, args);
    va_end(args);
    *m_os << buffer.data() << C_RST << std::endl;
}

void Logging::SetOutputStream(std::ostream* os)
{
    Logging::m_os = os;
    C_RED = "";
    C_YEL = "";
    C_CYN = "";
    C_RST = "";
}

void Logging::UnsetOutputStream()
{
    Logging::m_os = &std::cout;
}

void Logging::DisableLogging()
{
    Logging::m_bDisabled = true;
}

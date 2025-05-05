#ifndef __LOGGING_HXX__
#define __LOGGING_HXX__

#include <ostream>

enum class LogType
{
    INFO,
    WARN,
    ERROR
};

class Logging
{
public:
    static void Log(LogType type, const char* format, ...);
    static void LogInfo(const char* format, ...);
    static void LogWarn(const char* format, ...);
    static void LogError(const char* format, ...);
    static void SetOutputStream(std::ostream* os);
    static void UnsetOutputStream();
    static void DisableLogging();

private:
    static bool m_bDisabled;
    static std::ostream* m_os;
};

#endif // __LOGGING_HXX__

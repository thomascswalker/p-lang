#include "../Public/Logging.h"

Logging::Logger* Logging::Logger::Instance = GetInstance();

Logging::Logger* Logging::Logger::GetInstance()
{
    if (Instance == nullptr)
    {
        Instance = new Logger();
    }
    return Instance;
}

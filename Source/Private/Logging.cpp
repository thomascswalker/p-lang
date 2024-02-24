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

int Logging::Logger::GetCount(const LogLevel Level)
{
    int Count = 0;
    for (const auto& MsgLevel : Messages | std::views::values)
    {
        if (MsgLevel == Level)
        {
            Count++;
        }
    }
    return Count;
}
std::vector<std::string> Logging::Logger::GetMessages(const LogLevel Level)
{
    std::vector<std::string> Result;
    for (const auto& [Msg, MsgLevel] : Messages)
    {
        if (MsgLevel == Level)
        {
            Result.push_back(Msg);
        }
    }
    return Result;
}

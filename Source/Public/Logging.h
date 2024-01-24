#pragma once

#include <format>
#include <string>
#include <vector>


namespace Logging
{
    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error,
    };

    class Logger
    {
        std::vector<std::pair<std::string, LogLevel>> Messages;

        Logger() = default;

    public:
        static Logger*     Instance;
        static int         Line;
        static int         Column;
        static std::string Source;

        Logger(Logger& Other) = delete;
        ~Logger() = default;
        void operator=(const Logger& Other) = delete;

        static Logger* GetInstance();

        template <typename... Types>
        void Log(std::format_string<Types...> Fmt, LogLevel Level, Types&&... Args)
        {
            std::string Msg = std::format(Fmt, std::forward<Types>(Args)...);
            Messages.push_back({ Msg, Level });
        }

        int GetCount(const LogLevel Level)
        {
            int Count = 0;
            for (const auto& [Msg, MsgLevel] : Messages)
            {
                if (MsgLevel == Level)
                {
                    Count++;
                }
            }
            return Count;
        }

        std::vector<std::string> GetMessages(const LogLevel Level)
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
    };

    static Logger* GetLogger()
    {
        return Logger::GetInstance();
    }

    template <typename... Types>
    static constexpr void Debug(std::format_string<Types...> Fmt, Types&&... Args)
    {
        #ifdef _DEBUG
        std::cout << std::format(Fmt, std::forward<Types>(Args)...) << std::endl;
        Logger::GetInstance()->Log(Fmt, _Debug, std::forward<Types>(Args)...);
        #endif
    }

    template <typename... Types>
    static constexpr void Info(std::format_string<Types...> Fmt, Types&&... Args)
    {
        Logger::GetInstance()->Log(Fmt, LogLevel::Info, std::forward<Types>(Args)...);
    }

    template <typename... Types>
    static constexpr void Warning(std::format_string<Types...> Fmt, Types&&... Args)
    {
        Logger::GetInstance()->Log(Fmt, LogLevel::Warning, std::forward<Types>(Args)...);
    }

    template <typename... Types>
    static constexpr void Error(std::format_string<Types...> Fmt, Types&&... Args)
    {
        Logger::GetInstance()->Log(Fmt, LogLevel::Error, std::forward<Types>(Args)...);
    }
} // namespace Logging

static int         IndentDepth = 0;
static std::string GetIndent()
{
    return std::string(IndentDepth, ' '); // NOLINT(modernize-return-braced-init-list)
}

#ifdef _DEBUG
    #define DEBUG_ENTER                                             \
        Logging::Debug("{}Entering {}.", GetIndent(), __FUNCSIG__); \
        IndentDepth++;
    #define DEBUG_EXIT \
        IndentDepth--; \
        Logging::Debug("{}Exiting {}.", GetIndent(), __FUNCSIG__);
#else
#define DEBUG_ENTER
#define DEBUG_EXIT
#endif

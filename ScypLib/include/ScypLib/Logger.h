#pragma once
#include<string>
#include<iostream>
#include<functional>
#include<fstream>

#if defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#else
#define DEBUG_BREAK() ((void)0)
#endif

namespace sl

{
    enum class LogLevel
    {
        Info,
        Debug,
        Warning,
        Error,
    };

    class Logger
    {
    public:
        using LogFunction = std::function<void(LogLevel, const std::string&)>;

        static Logger& GetInstance()
        {
            static Logger instance;
            return instance;
        }

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;

        void SetLogFilePath(const std::string& filepath)
        {
            file = std::ofstream(filepath);
        }

        void SetLogCallback(LogFunction func)
        {
            logCallback = std::move(func);
        }

        void SetMinimumLogLevel(LogLevel level)
        {
            minimumLogLevel = level;
        }

        void Log(LogLevel level, const std::string& message)
        {
            if (level >= minimumLogLevel)
            {
                if (logCallback)
                {
                    logCallback(level, message);
                }
                else
                {
                    DefaultLog(level, message);
                }
            }
        }

    private:
        Logger() = default;
        ~Logger() = default;

        void DefaultLog(LogLevel level, const std::string& message)
        {
            std::string levelStr;
            const char* colorCode = nullptr;

            switch (level)
            {
            case LogLevel::Info:
                levelStr = "[Info]: ";
                colorCode = "\033[32m";
                break;
            case LogLevel::Debug:
                levelStr = "[Debug]: ";
                colorCode = "\033[34m";
                break;
            case LogLevel::Warning:
                levelStr = "[Warning]: ";
                colorCode = "\033[33m";
                break;
            case LogLevel::Error:
                levelStr = "[Error]: ";
                colorCode = "\033[31m";
                break;
            }

            std::cout << colorCode << levelStr << message << "\033[0m" << std::endl;

            if (file.is_open()) file << levelStr << message << std::endl;
        }

        LogFunction logCallback = nullptr;
        LogLevel minimumLogLevel = LogLevel::Debug;
        std::ofstream file;
    };
}
    #ifdef NDEBUG
    #define LOG_MESSAGE(level, msg) ((void)0)
    #else
    #define LOG_MESSAGE(level, msg)                             \
        do                                                      \
        {                                                       \
            sl::Logger::GetInstance().Log(level, msg);              \
            if ((level) == sl::LogLevel::Error) DEBUG_BREAK();  \
        } while (0)
    #define LOG_INFO(msg) LOG_MESSAGE(sl::LogLevel::Info, msg)
    #define LOG_DEBUG(msg) LOG_MESSAGE(sl::LogLevel::Debug, msg)
    #define LOG_WARN(msg) LOG_MESSAGE(sl::LogLevel::Warning, msg)
    #define LOG_ERROR(msg) LOG_MESSAGE(sl::LogLevel::Error, msg)
    #define LOG_FORMAT(level, fmt, ...)                         \
        do                                                      \
        {                                                       \
            char buf[1024];                                     \
            snprintf(buf, sizeof(buf), fmt, __VA_ARGS__);       \
            sl::Logger::GetInstance().Log(level, buf);              \
            if ((level) == sl::LogLevel::Error) DEBUG_BREAK();      \
        } while (0)
    #endif

#pragma once

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <string>

class Logger {
public:
    enum class Level { INFO, WARN, ERR };

    static void info(const char* fmt, ...) { log(Level::INFO, fmt); }
    static void warn(const char* fmt, ...) { log(Level::WARN, fmt); }
    static void error(const char* fmt, ...) { log(Level::ERR, fmt); }

private:
    static const char* level_name(Level level) {
        switch (level) {
            case Level::INFO: return "INFO";
            case Level::WARN: return "WARN";
            case Level::ERR:  return "ERROR";
        }
        return "INFO";
    }

    static void log(Level level, const char* fmt, ...) {
        const auto now = std::chrono::system_clock::now();
        const auto tt = std::chrono::system_clock::to_time_t(now);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::tm tm_buf{};
        localtime_r(&tt, &tm_buf);

        char time_str[32];
        std::snprintf(time_str, sizeof(time_str),
            "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
            tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
            tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec,
            static_cast<long>(ms.count()));

        std::va_list args;
        va_start(args, fmt);
        std::fprintf(stdout, "[%s] [%s] ", time_str, level_name(level));
        std::vfprintf(stdout, fmt, args);
        std::fprintf(stdout, "\n");
        va_end(args);
        std::fflush(stdout);
    }
};

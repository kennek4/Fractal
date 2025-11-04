#pragma once

#include "FTL_pch.h"

namespace FTL {
class Log {
  public:
    static void init();

    inline static std::shared_ptr<spdlog::logger> &errLogger() {
        return sErrLogger;
    };

    inline static std::shared_ptr<spdlog::logger> &stdLogger() {
        spdlog::set_default_logger(sStdLogger);
        return sStdLogger;
    };

    template <typename... Args>
    static void logError(fmt::format_string<Args...> fmt, Args &&...args) {
        spdlog::set_default_logger(sErrLogger);
        SPDLOG_ERROR(fmt, args...);
    };

    template <typename... Args>
    static void logCritical(fmt::format_string<Args...> fmt, Args &&...args) {
        spdlog::set_default_logger(sErrLogger);
        SPDLOG_CRITICAL(fmt, args...);
    };

  private:
    static std::shared_ptr<spdlog::logger> sErrLogger;
    static std::shared_ptr<spdlog::logger> sStdLogger;
};

}; // namespace FTL

// NOTE: Core Logging Macros
#define FTL_TRACE(...)                                                         \
    FTL::Log::stdLogger()->trace(__VA_ARGS__) // spdlog::trace(fmt)

#define FTL_DEBUG(...)                                                         \
    FTL::Log::stdLogger()->debug(__VA_ARGS__) // spdlog::debug(fmt)

#define FTL_INFO(...)                                                          \
    FTL::Log::stdLogger()->info(__VA_ARGS__) // spdlog::info(fmt)

#define FTL_WARN(...)                                                          \
    FTL::Log::stdLogger()->warn(__VA_ARGS__) // spdlog::warn(fmt)

#define FTL_ERROR(...) FTL::Log::logError(__VA_ARGS__) // spdlog::error(fmt)

#define FTL_CRITICAL(...)                                                      \
    FTL::Log::logCritical(__VA_ARGS__) // spdlog::critical(fmt)

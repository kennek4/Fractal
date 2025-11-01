#pragma once

#include "FTL_pch.h"
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace FTL {
class Log {
  public:
    static void init();

    static std::shared_ptr<spdlog::logger> &coreLogger() {
        return sCoreLogger;
    };

    static std::shared_ptr<spdlog::logger> &clientLogger() {
        return sClientLogger;
    };

  private:
    static std::shared_ptr<spdlog::logger> sCoreLogger;
    static std::shared_ptr<spdlog::logger> sClientLogger;
};

}; // namespace FTL

// NOTE: Core Logging Macros
#define FTL_CORE_TRACE(...)                                                    \
    FTL::Log::coreLogger()->trace(__VA_ARGS__) // spdlog::trace(fmt)

#define FTL_CORE_DEBUG(...)                                                    \
    FTL::Log::coreLogger()->debug(__VA_ARGS__) // spdlog::debug(fmt)

#define FTL_CORE_INFO(...)                                                     \
    FTL::Log::coreLogger()->info(__VA_ARGS__) // spdlog::info(fmt)

#define FTL_CORE_WARN(...)                                                     \
    FTL::Log::coreLogger()->warn(__VA_ARGS__) // spdlog::warn(fmt)

#define FTL_CORE_ERROR(...)                                                    \
    FTL::Log::coreLogger()->error(__VA_ARGS__) // spdlog::error(__VA_ARGS__)

#define FTL_CORE_CRITICAL(...)                                                 \
    FTL::Log::coreLogger()->critical(__VA_ARGS__) // spdlog::critical(fmt)

// NOTE: Client Side Logging Macros

#define FTL_TRACE(...)                                                         \
    FTL::Log::clientLogger()->trace(__VA_ARGS__) // spdlog::trace(fmt)

#define FTL_DEBUG(...) FTL::Log::debug(__VA_ARGS__) // spdlog::debug(fmt)

#define FTL_INFO(...)                                                          \
    FTL::Log::clientLogger()->info(__VA_ARGS__) // spdlog::info(fmt)

#define FTL_WARN(...)                                                          \
    FTL::Log::clientLogger()->warn(__VA_ARGS__) // spdlog::warn(fmt)

#define FTL_ERROR(...)                                                         \
    FTL::Log::error(__VA_ARGS__) // spdlog::error(__VA_ARGS__)

#define FTL_CRITICAL(...)                                                      \
    FTL::Log::clientLogger()->critical(__VA_ARGS__) // spdlog::critical(fmt)

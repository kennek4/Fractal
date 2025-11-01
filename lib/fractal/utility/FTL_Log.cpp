#include "FTL_Log.h"

namespace FTL {

std::shared_ptr<spdlog::logger> Log::sCoreLogger;
std::shared_ptr<spdlog::logger> Log::sClientLogger;

void Log::init() {
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

    logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "logs/Fractal.log", true));

    logSinks[0]->set_pattern("%^[%D] [%T.%f] [%n]: %v%$");
    sCoreLogger = std::make_shared<spdlog::logger>(
        "Fractal::Core", begin(logSinks), end(logSinks));
    sCoreLogger->set_level(spdlog::level::trace);
    sCoreLogger->flush_on(spdlog::level::trace);

    logSinks[1]->set_pattern("[%D] [%T.%f] [%l] [%n]: %v");
    sClientLogger = std::make_shared<spdlog::logger>(
        "Fractal::Application", begin(logSinks), end(logSinks));
    spdlog::register_logger(sClientLogger);
    sClientLogger->set_level(spdlog::level::trace);
    sClientLogger->flush_on(spdlog::level::trace);
};
}; // namespace FTL

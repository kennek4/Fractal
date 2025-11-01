#include "FTL_Log.h"

namespace FTL {

std::shared_ptr<spdlog::logger> Log::sErrLogger;
std::shared_ptr<spdlog::logger> Log::sStdLogger;

void Log::init() {
    const std::vector<spdlog::sink_ptr> sinks {
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
        std::make_shared<spdlog::sinks::stderr_color_sink_mt>(),
        std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/Fractal.log",
                                                            true)};

    const auto &stdSink  = sinks[0];
    const auto &errSink  = sinks[1];
    const auto &fileSink = sinks[2];

    stdSink->set_pattern("[%D] [%T.%f] %^[%n] %v%$");
    stdSink->set_level(spdlog::level::info);

    errSink->set_pattern("[%D] [%T.%f] %^[%n] [%s:%#] %v%$");
    errSink->set_level(spdlog::level::err);

    fileSink->set_pattern("[%D] [%T.%f] [%n] [%l] %v");
    fileSink->set_level(spdlog::level::trace);

    sErrLogger = std::make_shared<spdlog::logger>("FTL::Core", errSink);
    sErrLogger->set_level(spdlog::level::err);
    sErrLogger->flush_on(spdlog::level::err);

    sStdLogger =
        std::make_shared<spdlog::logger>("FTL::Core", begin(sinks), end(sinks));
    sStdLogger->set_level(spdlog::level::trace);
    sStdLogger->flush_on(spdlog::level::trace);
};
}; // namespace FTL

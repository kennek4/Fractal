/**
 * GTFO-Profiler
 * Google Trace (Event) Format Output Profiler
 *
 * Author: Xyle Ken Tabanay
 * GitHub: https://github.com/kennek4
 *
 * C++11 Standard Compliant
 */
#ifndef __GTFO_PROFILER_H
#define __GTFO_PROFILER_H

#ifndef GTFO_TIMESCALE_MS
#define GTFO_TIMESCALE "ms"
#endif

#ifdef GTFO_TIMESCALE_NS
#define GTFO_TIMESCALE "ns"
#endif

#ifndef __GTFO_JSON
#define __GTFO_JSON
#include "include/nlohmann/json.hpp"
#endif

#include <chrono>
#include <fstream>
#include <functional>
#include <thread>
#include <vector>

namespace GTFO {
struct ProfileResult {
    const char *name;
    const char *category;
    uint32_t threadId;
    long long start;
    long long end;
};

struct ProfileSession {
    const char *name;
};

using HighResTimePoint =
    std::chrono::time_point<std::chrono::high_resolution_clock>;

using JSON = nlohmann::json;

class Profiler {
  private:
    bool mIsActive;

    ProfileSession *mCurrentSession{nullptr};
    const char *mSessionOutputFile{"NULL"};
    const char *mSessionName{"NULL"};

    JSON mJSON;
    std::vector<JSON> mJSONData;
    std::ofstream mJSONFileStream;

    Profiler();
    ~Profiler();

  public:
    static Profiler &get() {
        static Profiler instance;
        return instance;
    };

    void startSession(const char *name,
                      const char *filePath = "sessionResults.json");

    void endSession();
    void writeProfile(const ProfileResult &result);
};

class Timer {
  private:
    HighResTimePoint mStartTimePoint;
    ProfileResult mResult;
    bool mIsFinished;

  public:
    explicit Timer(const char *name, const char *category = "function");
    ~Timer();

    void stop();
};
}; // namespace GTFO

#ifndef GTFO_PROFILER_OFF
#define GTFO_PROFILE_SESSION_START(name, filePath)                             \
    GTFO::Profiler::get().startSession(name, filePath);
#define GTFO_PROFILE_SESSION_END() GTFO::Profiler::get().endSession();
#define GTFO_PROFILE_SCOPE(scopeName, scopeCategory)                           \
    GTFO::Timer timer##__LINE__(scopeName, scopeCategory)
#define GTFO_PROFILE_FUNCTION() GTFO_PROFILE_SCOPE(__FUNCTION__, "function")
#endif

#ifdef GTFO_PROFILER_OFF
#define GTFO_PROFILE_SESSION_START(name, filePath)
#define GTFO_PROFILE_SESSION_END()
#define GTFO_PROFILE_SCOPE(scopeName, scopeCategory)
#define GTFO_PROFILE_FUNCTION() GTFO_PROFILE_SCOPE(__FUNCTION__, "function")
#endif

#endif // END OF __GTFO_PROFILER_H

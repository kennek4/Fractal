#include "gtfo_profiler.h"

namespace GTFO {

Profiler::Profiler() {
    // HACK: Arbitrary space reservation
    mJSONData.reserve(1024);
};

Profiler::~Profiler() {
    if (mIsActive)
        endSession();
};

void Profiler::startSession(const char *name, const char *filePath) {
    if (mIsActive)
        endSession();

    mSessionOutputFile = filePath;
    mSessionName = name;

    mJSONFileStream.open(mSessionOutputFile);
    mIsActive = true;
};

void Profiler::endSession() {
    if (!mIsActive)
        return;

    mJSON = mJSONData;
    mJSONFileStream << mJSON;
    mJSONFileStream.close();

    mJSONData.clear();
    mIsActive = false;
};

void Profiler::writeProfile(const ProfileResult &result) {
    JSON profile;

    profile["name"] = result.name;
    profile["cat"] = result.category;
    profile["ph"] = "X";
    profile["ts"] = result.start;
    profile["dur"] = (result.end - result.start);
    profile["pid"] = 0;
    profile["tid"] = result.threadId;

    mJSONData.emplace_back(profile);
};

Timer::Timer(const char *name, const char *category) {
    mStartTimePoint = std::chrono::high_resolution_clock::now();

    mResult.name = name;
    mResult.category = category;
    mResult.start = 0;
    mResult.end = 0;
    mResult.threadId = std::hash<std::thread::id>()(std::this_thread::get_id());

    mIsFinished = false;
};

Timer::~Timer() {
    if (!mIsFinished)
        stop();
}

void Timer::stop() {
    HighResTimePoint endTimePoint = std::chrono::high_resolution_clock::now();

    mResult.start =
        std::chrono::time_point_cast<std::chrono::microseconds>(mStartTimePoint)
            .time_since_epoch()
            .count();

    mResult.end =
        std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint)
            .time_since_epoch()
            .count();

    Profiler::get().writeProfile(mResult);
    mIsFinished = true;
};

} // namespace GTFO

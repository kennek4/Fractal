#include "gtfo_profiler.h"
#include <Fractal.h>

#ifdef __FRACTAL_PLATFORM_LINUX
int main(int argc, char *argv[]) {
    FTL::Application *app = new FTL::Application();

    GTFO_PROFILE_SESSION_START("AppInit", "logs/FTLAppInit.log");
    app->init();
    GTFO_PROFILE_SESSION_END();

    GTFO_PROFILE_SESSION_START("AppRun", "logs/FTLAppRun.log");
    app->run();
    GTFO_PROFILE_SESSION_END();

    GTFO_PROFILE_SESSION_START("AppShutdown", "logs/FTLAppShutdown.log");
    delete app;
    GTFO_PROFILE_SESSION_END();

    return EXIT_SUCCESS;
};
#endif
#ifdef __FRACTAL_PLATFORM_WIN32
int main(int argc, char *argv[]) {
    FTL::HelloWorld();
    return EXIT_SUCCESS;
}
#endif

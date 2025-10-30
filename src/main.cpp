#include <Fractal.h>

#ifdef __FRACTAL_PLATFORM_LINUX
int main(int argc, char *argv[]) {
    FTL::Application *app = new FTL::Application();

    app->run();

    delete app;
    return EXIT_SUCCESS;
}
#endif
#ifdef __FRACTAL_PLATFORM_WIN32

int main(int argc, char *argv[]) {
    FTL::HelloWorld();
    return EXIT_SUCCESS;
}

#endif

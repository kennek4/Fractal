#include "FTL_Application.h"
#include "gtfo_profiler.h"
#include <utility/FTL_Log.h>

namespace FTL {
Application::Application() {
    mWindowData = WindowData {.name = "Fractal", .width = 1920, .height = 1080};
    mRenderer   = std::make_unique<Renderer>();
};

Application::~Application() { mRenderer->shutdown(&mWindowData.window); };

void Application::init() {
    Log::init();

    {
        FTL_CORE_DEBUG("Creating vkInstance...");
        mRenderer->createInstance(&mWindowData);
    }
};

void Application::run() {
    while (!glfwWindowShouldClose(mWindowData.window)) {
        glfwPollEvents();
    }
};
}; // namespace FTL

#include "FTL_Application.h"

namespace FTL {
Application::Application() {
    mWindow = Window({"Fractal", 1920, 1080});
    mWindow.init();

    mRenderer = Renderer();
    mRenderer.init();
};

Application::~Application() {
    mRenderer.shutdown();
    mWindow.shutdown();
};

void Application::run() {
    while (!glfwWindowShouldClose(mWindow.get())) {
        glfwPollEvents();
    }
};
}; // namespace FTL

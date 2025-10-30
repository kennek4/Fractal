#include "FTL_Window.h"

namespace FTL {
Window::Window(WindowProperties winProps) { mWinProps = winProps; };

Window::~Window() {

};

bool Window::init() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(mWinProps.width, mWinProps.height,
                               mWinProps.name, nullptr, nullptr);
    return true;
};

bool Window::shutdown() {
    glfwDestroyWindow(mWindow);
    glfwTerminate();
    return true;
};

}; // namespace FTL

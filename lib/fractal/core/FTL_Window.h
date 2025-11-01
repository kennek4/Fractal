#pragma once

#include <utility/FTL_pch.h>

namespace FTL {
struct WindowData {
    GLFWwindow *window;
    const char *name {"DefaultWindowName"};
    uint32_t width {800};
    uint32_t height {600};
};
}; // namespace FTL

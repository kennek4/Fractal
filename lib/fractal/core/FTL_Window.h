#pragma once

#include "FTL_System.h"
#include <utility/FTL_pch.h>

namespace FTL {
struct WindowProperties {
    const char *name{"Default Window Name"};
    uint32_t width{400};
    uint32_t height{300};
};

class Window : public System {
  private:
    GLFWwindow *mWindow{nullptr};
    WindowProperties mWinProps{};

  public:
    Window(WindowProperties winProps = {});
    ~Window();

    GLFWwindow *get() const { return mWindow; };

    virtual bool init() override;
    virtual bool shutdown() override;
};
}; // namespace FTL

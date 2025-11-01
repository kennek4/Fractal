#pragma once

#include "FTL_Window.h"
#include <memory>
#include <renderer/FTL_Renderer.h>
#include <utility/FTL_pch.h>

namespace FTL {

class Application {
  private:
    WindowData mWindowData;
    std::unique_ptr<Renderer> mRenderer;

  public:
    Application();
    ~Application();

    void init();
    void run();
};

}; // namespace FTL

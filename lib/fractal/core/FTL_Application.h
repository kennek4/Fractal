#pragma once

#include "FTL_Window.h"
#include <renderer/FTL_Renderer.h>
#include <utility/FTL_pch.h>

namespace FTL {

class Application {
  private:
    Window mWindow;
    Renderer mRenderer;

  public:
    Application();
    ~Application();

    void run();
};

}; // namespace FTL

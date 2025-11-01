#pragma once

#include <core/FTL_Window.h>
#include <utility/FTL_pch.h>

namespace FTL {

struct VulkanCore {
    vk::raii::Context context;
    vk::raii::Instance instance {nullptr};
};

class Renderer {
  private:
    VulkanCore mVKCore;

  public:
    Renderer();
    ~Renderer();

    void shutdown(GLFWwindow **ppWindow);
    void createInstance(WindowData *pWinData);
};
}; // namespace FTL

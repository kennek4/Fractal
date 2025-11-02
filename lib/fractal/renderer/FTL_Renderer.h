#pragma once

#include <core/FTL_Window.h>
#include <utility/FTL_Log.h>
#include <utility/FTL_pch.h>

namespace FTL {

struct VulkanCore {};

class Renderer {
  private:
    vk::raii::Context mContext;
    vk::raii::Instance mInstance {nullptr};
    vk::raii::DebugUtilsMessengerEXT mDebugMessenger {nullptr};
    vk::raii::PhysicalDevice mPhysicalDevice {nullptr};
    vk::raii::Device mDevice {nullptr};
    vk::raii::Queue mQueue {nullptr};

  public:
    Renderer();
    ~Renderer();

    void shutdown(GLFWwindow **ppWindow);
    void init(WindowData *pWinData) {
        createInstance(pWinData);
        setupDebugMessenger();
        pickPhysicalDevice();
        createLogicalDevice();
    };

    void createInstance(WindowData *pWinData);
    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();
};
}; // namespace FTL

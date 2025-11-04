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
    vk::raii::SurfaceKHR mSurface {nullptr};
    vk::raii::PhysicalDevice mPhysicalDevice {nullptr};
    vk::raii::Device mDevice {nullptr};
    vk::raii::Queue mGraphicsQueue {nullptr};
    vk::raii::Queue mPresentQueue {nullptr};

    vk::raii::SwapchainKHR mSwapChain {nullptr};
    vk::Format mSwapChainFormat {vk::Format::eUndefined};
    vk::Extent2D mSwapChainExtent;
    std::vector<vk::Image> mSwapChainImages {};
    std::vector<vk::raii::ImageView> mSwapChainImageViews {};

    vk::raii::PipelineLayout mPipelineLayout {nullptr};
    vk::raii::Pipeline mGraphicsPipeline {nullptr};

  public:
    Renderer();
    ~Renderer();

    void shutdown(GLFWwindow **ppWindow);
    void init(WindowData *pWinData) {
        createInstance(pWinData);
        setupDebugMessenger();
        createSurface(pWinData->window);
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain(pWinData->window);
        createImageViews();
        createGraphicsPipeline();
    };

    void createInstance(WindowData *pWinData);
    void setupDebugMessenger();
    void createSurface(GLFWwindow *pWindow);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain(GLFWwindow *pWindow);
    void createImageViews();
    void createGraphicsPipeline();
};
}; // namespace FTL

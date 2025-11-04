#pragma once

#include "FTL_Vertex.h"
#include "gtfo_profiler.h"
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
    uint32_t mGraphicsQueueIndex;

    vk::raii::Queue mPresentQueue {nullptr};
    uint32_t mPresentQueueIndex;

    vk::raii::SwapchainKHR mSwapChain {nullptr};
    vk::Format mSwapChainFormat {vk::Format::eUndefined};
    vk::Extent2D mSwapChainExtent;
    std::vector<vk::Image> mSwapChainImages {};
    std::vector<vk::raii::ImageView> mSwapChainImageViews {};

    vk::raii::PipelineLayout mPipelineLayout {nullptr};
    vk::raii::Pipeline mGraphicsPipeline {nullptr};

    vk::raii::CommandPool mCommandPool {nullptr};
    vk::raii::CommandBuffer mCommandBuffer {nullptr};

    vk::raii::Semaphore mSemaphorePresentComplete {nullptr};
    vk::raii::Semaphore mSemaphoreRenderFinished {nullptr};
    vk::raii::Fence mFenceDraw {nullptr};

    vk::raii::Buffer mVertexBuffer {nullptr};
    vk::raii::DeviceMemory mVertexBufferMemory {nullptr};

    void createInstance(WindowData *pWinData);
    void setupDebugMessenger();
    void createSurface(GLFWwindow *pWindow);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain(GLFWwindow *pWindow);
    void createImageViews();
    void createGraphicsPipeline();
    void createCommandPool();
    void createVertexBuffer();
    void createCommandBuffer();
    void createSyncObjects();

    void recordCommandBuffer(uint32_t imageIndex);
    void transitionImageLayout(uint32_t imageIndex, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout,
                               vk::AccessFlags2 srcAccessMask,
                               vk::AccessFlags2 dstAccessMask,
                               vk::PipelineStageFlags2 srcStageMask,
                               vk::PipelineStageFlags2 dstStageMask);

  public:
    Renderer();
    ~Renderer();

    void shutdown(GLFWwindow **ppWindow);
    void init(WindowData *pWinData) {
        GTFO_PROFILE_SCOPE("Vulkan Init", "init");
        createInstance(pWinData);
        setupDebugMessenger();
        createSurface(pWinData->window);
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain(pWinData->window);
        createImageViews();
        createGraphicsPipeline();
        createCommandPool();
        createVertexBuffer();
        createCommandBuffer();
        createSyncObjects();
    };

    void render();
    void waitIdle() const { mDevice.waitIdle(); };
};
}; // namespace FTL

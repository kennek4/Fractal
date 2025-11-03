#include "FTL_Renderer.h"
#include "FTL_Log.h"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <spdlog/common.h>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_hpp_macros.hpp>
#include <vulkan/vulkan_raii.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace {
std::vector<const char *>
getRequiredExtensions(const bool hasValidationLayerSupport) {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (hasValidationLayerSupport) {
        FTL_DEBUG("Pushing EXTDebugUtilsExtensionName to requiredExtensions");
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
};

uint32_t getQueueFamilyIndex(vk::PhysicalDevice physicalDevice) {
    std::vector<vk::QueueFamilyProperties> qfps =
        physicalDevice.getQueueFamilyProperties();

    auto graphicsQueueFamilyProperty = std::find_if(
        qfps.begin(), qfps.end(), [](vk::QueueFamilyProperties const &qfp) {
            return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
        });

    return static_cast<uint32_t>(
        std::distance(qfps.begin(), graphicsQueueFamilyProperty));
};

vk::SurfaceFormatKHR getSwapChainSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
    for (const vk::SurfaceFormatKHR &format : availableFormats) {
        if (format.format == vk::Format::eB8G8R8A8Srgb &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        };
    };

    return availableFormats[0];
};

vk::PresentModeKHR
getSwapChainPresentMode(const std::vector<vk::PresentModeKHR> &availableModes) {
    for (const vk::PresentModeKHR mode : availableModes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            return mode;
        };
    };

    return vk::PresentModeKHR::eFifo;
};

vk::Extent2D getSwapChainExtent(const vk::SurfaceCapabilitiesKHR &capabilities,
                                GLFWwindow *pWindow) {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    };

    int width, height;
    glfwGetFramebufferSize(pWindow, &width, &height);

    return {
        .width = std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                                      capabilities.maxImageExtent.width),
        .height =
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                                 capabilities.maxImageExtent.height),
    };
};

static VKAPI_ATTR vk::Bool32 VKAPI_CALL vkDebugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {

    FTL_DEBUG("[vkValidation:{}] {}", vk::to_string(type),
              pCallbackData->pMessage);

    return vk::False;
}

}; // namespace

namespace FTL {
Renderer::Renderer() {};

Renderer::~Renderer() {

};

const std::vector<const char *> ValidationLayers {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> RequiredDeviceExtensions {
    vk::KHRSwapchainExtensionName, vk::KHRSpirv14ExtensionName,
    vk::KHRSynchronization2ExtensionName,
    vk::KHRCreateRenderpass2ExtensionName};

#ifndef NDEBUG
constexpr bool hasValidationLayerSupport = true;
#else
constexpr bool hasValidationLayerSupport = false;
#endif

void Renderer::createInstance(WindowData *pWinData) {
    GTFO_PROFILE_FUNCTION();

    {
        GTFO_PROFILE_SCOPE("glfwInit() & glfwCreateWindow()", "scope");
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        pWinData->window = glfwCreateWindow(pWinData->width, pWinData->height,
                                            pWinData->name, nullptr, nullptr);
    }

    const vk::ApplicationInfo appInfo {
        .pApplicationName   = "Fractal",
        .applicationVersion = vk::makeVersion(0, 1, 0),
        .pEngineName        = "No Engine",
        .engineVersion      = vk::makeVersion(0, 1, 0),
        .apiVersion         = vk::ApiVersion14};

    std::vector<const char *> requiredLayers;
    if (hasValidationLayerSupport) {
        requiredLayers.assign(ValidationLayers.begin(), ValidationLayers.end());
    };

    auto layerProperties = mContext.enumerateInstanceLayerProperties();
    auto _layerPredicate = [&layerProperties](auto const &requiredLayer) {
        return std::ranges::none_of(
            layerProperties, [requiredLayer](auto const &layerProperty) {
                return strcmp(layerProperty.layerName, requiredLayer) == 0;
            });
    };

    if (std::ranges::any_of(requiredLayers, _layerPredicate)) {
        throw std::runtime_error("[Fractal/Renderer] One or more required "
                                 "layers are not supported!");
    };

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = nullptr;
    {
        GTFO_PROFILE_SCOPE("Getting GLFW Required Extensions", "scope");
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    }

    std::vector<const char *> requiredExtensions;
    {
        GTFO_PROFILE_SCOPE("Getting Required Extensions", "scope");
        requiredExtensions = getRequiredExtensions(hasValidationLayerSupport);
    }

    auto extensionProperties = mContext.enumerateInstanceExtensionProperties();

    {
        GTFO_PROFILE_SCOPE("Required Extensions Check", "scope");
        for (auto const &requiredExtension : requiredExtensions) {
            auto predicate =
                [requiredExtension](auto const &extensionProperty) {
                    return strcmp(extensionProperty.extensionName,
                                  requiredExtension) == 0;
                };

            if (std::ranges::none_of(extensionProperties, predicate)) {
                throw std::runtime_error("Required extension not supported: " +
                                         std::string(requiredExtension));
            }
        };
    }

    {
        GTFO_PROFILE_SCOPE("Required GLFW Extensions Check", "scope");
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            auto predicate = [glfwExtension = glfwExtensions[i]](
                                 auto const &extensionProperty) {
                return strcmp(extensionProperty.extensionName, glfwExtension);
            };

            if (std::ranges::none_of(extensionProperties, predicate)) {
                throw std::runtime_error(
                    "[Fractal/Renderer] GLFW requires the following"
                    "extension but it is not supported: " +
                    std::string(glfwExtensions[i]));
            };
        };
    }

    const vk::InstanceCreateInfo createInfo {
        .pApplicationInfo    = &appInfo,
        .enabledLayerCount   = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount =
            static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data(),
    };

    {
        GTFO_PROFILE_SCOPE("vk::raii::Instance Call", "init");
        mInstance = vk::raii::Instance(mContext, createInfo);
    }
};

void Renderer::setupDebugMessenger() {
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    vk::DebugUtilsMessengerCreateInfoEXT createInfo {
        .messageSeverity = severityFlags,
        .messageType     = messageTypeFlags,
        .pfnUserCallback = &vkDebugCallback};

    mDebugMessenger = mInstance.createDebugUtilsMessengerEXT(createInfo);
};

void Renderer::createSurface(GLFWwindow *pWindow) {
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*mInstance, pWindow, nullptr, &_surface) !=
        VK_SUCCESS) {
        constexpr const char *errMsg = "Failed to create a Vulkan Surface";
        FTL_CRITICAL(errMsg);
        throw std::runtime_error(errMsg);
    }

    mSurface = vk::raii::SurfaceKHR(mInstance, _surface);
    FTL_DEBUG("Successfully created a Vulkan Surface! ");
};

void Renderer::pickPhysicalDevice() {
    std::vector<vk::raii::PhysicalDevice> devices =
        mInstance.enumeratePhysicalDevices();

    if (devices.empty())
        throw std::runtime_error("Failed to find Vulkan Supported GPUs!");

    bool hasVulkan14Support;
    bool hasRequiredProperties;
    bool hasRequiredFeatures;

    vk::raii::PhysicalDevice device {nullptr};
    for (uint32_t i = 0; i < devices.size(); i++) {
        device                = devices[i];
        auto deviceProperties = device.getProperties();
        auto deviceFeatures   = device.getFeatures();
        bool isValidDevice    = deviceProperties.apiVersion >= vk::ApiVersion14;

        auto qfps             = device.getQueueFamilyProperties();
        auto graphicsQfpPredicate = [](vk::QueueFamilyProperties const &qfp) {
            return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) !=
                   static_cast<vk::QueueFlags>(0);
        };

        const auto qfpIter = std::ranges::find_if(qfps, graphicsQfpPredicate);
        isValidDevice      = isValidDevice && (qfpIter != qfps.end());

        auto deviceExtensions = device.enumerateDeviceExtensionProperties();
        for (const char *const &deviceExtension : RequiredDeviceExtensions) {
            auto predicate = [deviceExtension](auto const &ext) {
                return strcmp(ext.extensionName, deviceExtension) == 0;
            };

            auto extIter = std::ranges::find_if(deviceExtensions, predicate);
            isValidDevice =
                isValidDevice && (extIter != deviceExtensions.end());
        };

        if (!isValidDevice) {
            continue;
        }

        FTL_DEBUG("A valid physical device was found!");
        mPhysicalDevice = device;
        return;
    };

    FTL_CRITICAL("No valid physical devices were found!");
    throw std::runtime_error("No valid physical devices were found!");
};

void Renderer::createLogicalDevice() {
    // find the index of the first queue family that supports graphics
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
        mPhysicalDevice.getQueueFamilyProperties();

    // get the first index into queueFamilyProperties which supports graphics
    auto graphicsQueueFamilyProperty =
        std::ranges::find_if(queueFamilyProperties, [](auto const &qfp) {
            return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) !=
                   static_cast<vk::QueueFlags>(0);
        });
    assert(graphicsQueueFamilyProperty != queueFamilyProperties.end() &&
           "No graphics queue family found!");

    uint32_t graphicsIndex = static_cast<uint32_t>(std::distance(
        queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

    uint32_t presentIndex =
        mPhysicalDevice.getSurfaceSupportKHR(graphicsIndex, mSurface)
            ? graphicsIndex
            : static_cast<uint32_t>(queueFamilyProperties.size());

    if (presentIndex == queueFamilyProperties.size()) {
        FTL_DEBUG("Finding Present Queue Index...");
        bool hasGraphicsSupport = true;
        bool hasPresentSupport  = true;

        for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
            hasGraphicsSupport =
                static_cast<bool>(queueFamilyProperties[i].queueFlags &
                                  vk::QueueFlagBits::eGraphics);
            hasGraphicsSupport = mPhysicalDevice.getSurfaceSupportKHR(
                static_cast<uint32_t>(i), mSurface);

            if (hasGraphicsSupport && hasPresentSupport) {
                graphicsIndex = static_cast<uint32_t>(i);
                presentIndex  = graphicsIndex;
                FTL_DEBUG("Graphics Queue and Present Queue have the same "
                          "indicies: {}, {}  respectively",
                          graphicsIndex, presentIndex);
                break;
            } else if (!hasGraphicsSupport && hasPresentSupport) {
                presentIndex = static_cast<uint32_t>(i);
                FTL_DEBUG("Graphics Queue and Present Queue have different "
                          "indicies: {}, {}  respectively",
                          graphicsIndex, presentIndex);
                break;
            }
        };

        if (graphicsIndex == queueFamilyProperties.size() ||
            presentIndex == queueFamilyProperties.size()) {
            constexpr const char *errMsg = "No valid Vulkan Queue was found "
                                           "that supports graphics or present!";
            FTL_CRITICAL(errMsg);
            throw std::runtime_error(errMsg);
        };
    };

    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain {
            {}, // vk::PhysicalDeviceFeatures2 (empty for now)
            {.dynamicRendering =
                 true}, // Enable dynamic rendering from Vulkan 1.3
            {.extendedDynamicState =
                 true} // Enable extended dynamic state from the extension
        };

    float queuePriority = 0.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
        .queueFamilyIndex = graphicsIndex,
        .queueCount       = 1,
        .pQueuePriorities = &queuePriority,
    };

    vk::DeviceCreateInfo deviceCreateInfo {
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos    = &deviceQueueCreateInfo,
        .enabledExtensionCount =
            static_cast<uint32_t>(RequiredDeviceExtensions.size()),
        .ppEnabledExtensionNames = RequiredDeviceExtensions.data()};

    mDevice = vk::raii::Device(mPhysicalDevice, deviceCreateInfo);
    FTL_DEBUG("Created the Vulkan Logical Device!");

    mGraphicsQueue = vk::raii::Queue(mDevice, graphicsIndex, 0);
    FTL_DEBUG("Created the Vulkan Graphics Queue!");

    mPresentQueue = vk::raii::Queue(mDevice, presentIndex, 0);
    FTL_DEBUG("Created the Vulkan Present Queue!");
};

void Renderer::createSwapChain(GLFWwindow *pWindow) {
    vk::SurfaceCapabilitiesKHR surfaceCapabilites =
        mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);

    vk::SurfaceFormatKHR surfaceFormat = getSwapChainSurfaceFormat(
        mPhysicalDevice.getSurfaceFormatsKHR(mSurface));

    mSwapChainFormat       = surfaceFormat.format;
    mSwapChainExtent       = getSwapChainExtent(surfaceCapabilites, pWindow);

    uint32_t minImageCount = std::max(3u, surfaceCapabilites.minImageCount);
    if (surfaceCapabilites.maxImageCount > 0 &&
        minImageCount > surfaceCapabilites.minImageCount) {
        minImageCount = surfaceCapabilites.maxImageCount;
    };

    vk::SwapchainCreateInfoKHR createInfo {
        .flags            = vk::SwapchainCreateFlagsKHR(),
        .surface          = mSurface,
        .minImageCount    = minImageCount,
        .imageFormat      = surfaceFormat.format,
        .imageColorSpace  = surfaceFormat.colorSpace,
        .imageExtent      = mSwapChainExtent,
        .imageArrayLayers = 1,
        .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform     = surfaceCapabilites.currentTransform,
        .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode      = getSwapChainPresentMode(
            mPhysicalDevice.getSurfacePresentModesKHR(mSurface)),
        .clipped      = true,
        .oldSwapchain = VK_NULL_HANDLE};

    mSwapChain       = vk::raii::SwapchainKHR(mDevice, createInfo);
    mSwapChainImages = mSwapChain.getImages();
    FTL_DEBUG("Vulkan Swap Chain created successfully!");
};

void Renderer::createImageViews() {
    mSwapChainImageViews.clear();

    vk::ImageViewCreateInfo createInfo {
        .viewType         = vk::ImageViewType::e2D,
        .format           = mSwapChainFormat,
        .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    };

    for (vk::Image image : mSwapChainImages) {
        createInfo.image = image;
        mSwapChainImageViews.emplace_back(mDevice, createInfo);
    }

    FTL_DEBUG("Vulkan Image Views created successfully!");
};

void Renderer::shutdown(GLFWwindow **ppWindow) {
    glfwDestroyWindow(*ppWindow);
    glfwTerminate();
};
}; // namespace FTL

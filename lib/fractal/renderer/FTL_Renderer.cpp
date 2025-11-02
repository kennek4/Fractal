#include "FTL_Renderer.h"
#include "FTL_Log.h"
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

    float queuePriority = 1.0f;
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

    mQueue = vk::raii::Queue(mDevice, graphicsIndex, 0);
    FTL_DEBUG("Created the Vulkan Graphics Queue!");
};

void Renderer::shutdown(GLFWwindow **ppWindow) {
    glfwDestroyWindow(*ppWindow);
    glfwTerminate();
};
}; // namespace FTL

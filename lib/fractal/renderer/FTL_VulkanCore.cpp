#include "FTL_VulkanCore.h"

namespace FTL {

static VKAPI_ATTR VkBool32 VKAPI_CALL
vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData) {
    // TODO: Custom logging here!
    std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
};

VkDebugUtilsMessengerCreateInfoEXT initDebugMessengerCreateInfo() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    createInfo.pfnUserCallback = vkDebugCallback;
    return createInfo;
};

bool hasValidationLayerSupport() {
    uint32_t layerCount{0};
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *validationLayerName : validationLayers) {
        bool isLayerAvailable = false;
        for (const auto &availableLayer : availableLayers) {
            if (strcmp(validationLayerName, availableLayer.layerName) == 0) {
                isLayerAvailable = true;
                break;
            };
        };

        if (!isLayerAvailable) {
            return false;
        };
    };

    return true;
};

std::vector<const char *> getRequiredExtensions() {
    uint32_t extensionCount;
    const char **glfwExtensionNames =
        glfwGetRequiredInstanceExtensions(&extensionCount);

    std::vector<const char *> extensionNames(
        glfwExtensionNames, glfwExtensionNames + extensionCount);

    if (hasValidationLayers) {
        extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    };

    return extensionNames;
};

void createInstance(VkInstance *pInstance) {
    if (hasValidationLayers && !hasValidationLayerSupport()) {
        throw std::runtime_error("[Fractal/VulkanCore] Validation Layers were "
                                 "requested but are not available!");
    };

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_4;

    // TODO: Make these depend on build variables
    appInfo.pApplicationName = "Fractal";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);

    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (hasValidationLayers) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        auto debugCreateInfo = initDebugMessengerCreateInfo();
        createInfo.pNext =
            static_cast<VkDebugUtilsMessengerCreateInfoEXT *>(&debugCreateInfo);

    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    };

    if (vkCreateInstance(&createInfo, nullptr, pInstance) != VK_SUCCESS) {
        throw std::runtime_error(
            "[Fractal/VulkanCore] Failed to Create Vulkan Instance!");
    };
};

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (!func) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    };

    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
};

void setupDebugMessenger(const VkInstance &instance,
                         VkDebugUtilsMessengerEXT *pDebugMessenger) {
    if (!hasValidationLayers) {
        return;
    };

    auto debugCreateInfo = initDebugMessengerCreateInfo();
    if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr,
                                     pDebugMessenger) != VK_SUCCESS) {
        throw std::runtime_error(
            "[Fractal/VulkanCore] Failed to setup Debug Messanger!");
    };
};
} // namespace FTL

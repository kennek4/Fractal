#include "FTL_Renderer.h"
#include "FTL_Window.h"
#include "gtfo_profiler.h"
#include <algorithm>
#include <vector>
namespace FTL {
Renderer::Renderer() {

};

Renderer::~Renderer() {

};

const std::vector<const char *> ValidationLayers {
    "VK_LAYER_KHRONOS_validation"};

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

    auto layerProperties = mVKCore.context.enumerateInstanceLayerProperties();
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
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    auto extensionProperties =
        mVKCore.context.enumerateInstanceExtensionProperties();

    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        auto predicate =
            [glfwExtension = glfwExtensions[i]](auto const &extensionProperty) {
                return strcmp(extensionProperty.extensionName, glfwExtension);
            };

        if (std::ranges::none_of(extensionProperties, predicate)) {
            throw std::runtime_error(
                "[Fractal/Renderer] GLFW requires the following"
                "extension but it is not supported: " +
                std::string(glfwExtensions[i]));
        };
    };

    const vk::InstanceCreateInfo createInfo {
        .pApplicationInfo        = &appInfo,
        .enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames     = requiredLayers.data(),
        .enabledExtensionCount   = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions,
    };

    auto extensions = mVKCore.context.enumerateInstanceExtensionProperties();

    std::println("Available Extensions:\n");
    for (const auto &extension : extensions) {
        std::println("\t{}", std::string_view(extension.extensionName));
    };

    mVKCore.instance = vk::raii::Instance(mVKCore.context, createInfo);
};

void Renderer::shutdown(GLFWwindow **ppWindow) {
    glfwDestroyWindow(*ppWindow);
    glfwTerminate();
};
}; // namespace FTL

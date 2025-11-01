#include "FTL_Renderer.h"
#include "gtfo_profiler.h"

namespace {

std::vector<const char *>
getRequiredExtensions(const bool hasValidationLayerSupport) {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (hasValidationLayerSupport) {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
};

}; // namespace

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

    auto extensionProperties =
        mVKCore.context.enumerateInstanceExtensionProperties();

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
        .pApplicationInfo        = &appInfo,
        .enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames     = requiredLayers.data(),
        .enabledExtensionCount   = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions,
    };

    {
        GTFO_PROFILE_SCOPE("vk::raii::Instance Call", "init");
        mVKCore.instance = vk::raii::Instance(mVKCore.context, createInfo);
    }
};

void Renderer::shutdown(GLFWwindow **ppWindow) {
    glfwDestroyWindow(*ppWindow);
    glfwTerminate();
};
}; // namespace FTL

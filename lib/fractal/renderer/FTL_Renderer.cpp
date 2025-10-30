#include "FTL_Renderer.h"
#include "FTL_VulkanCore.h"
#include <vulkan/vulkan_core.h>

namespace FTL {
Renderer::Renderer() {

};

Renderer::~Renderer() {

};

bool Renderer::init() {
    bool isSuccess = true;

    createInstance(&mVkCore.instance);
    setupDebugMessenger(mVkCore.instance, &mVkCore.debugMessenger);

    return isSuccess;
};

bool Renderer::shutdown() {
    bool isSuccess = true;
    if (hasValidationLayers) {
        DestroyDebugUtilsMessengerEXT(mVkCore.instance, mVkCore.debugMessenger,
                                      nullptr);
    };

    vkDestroyInstance(mVkCore.instance, nullptr);
    return isSuccess;
};

}; // namespace FTL

#pragma once

#include <utility/FTL_pch.h>

namespace FTL {

constexpr std::array<const char *, 1> validationLayers{
    "VK_LAYER_KHRONOS_validation"};

#ifdef __FRACTAL_BUILD_DEBUG
const bool hasValidationLayers = true;
#else
const bool hasValidationLayers = false;
#endif

bool hasValidationLayerSupport();

void createInstance(VkInstance *pInstance);
void setupDebugMessenger(const VkInstance &instance,
                         VkDebugUtilsMessengerEXT *pDebugMessenger);
void createSurface();

}; // namespace FTL

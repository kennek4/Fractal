#pragma once

#include "FTL_VulkanCore.h"
#include <core/FTL_System.h>
#include <utility/FTL_pch.h>

namespace FTL {

struct VulkanCore {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
};

class Renderer : public System {
  private:
    VulkanCore mVkCore;

    void
    DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                  VkDebugUtilsMessengerEXT debugMessenger,
                                  const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func) {
            return func(instance, debugMessenger, pAllocator);
        };
    };

  public:
    Renderer();
    ~Renderer();

    virtual bool init() override;
    virtual bool shutdown() override;
};
}; // namespace FTL

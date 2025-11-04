#include "FTL_Renderer.h"
#include "FTL_Log.h"
#include "FTL_Vertex.h"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace {
static std::vector<char> readFile(const std::string &fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        constexpr const char *errMsg = "Failed to open file!";
        FTL_ERROR(errMsg);
        throw std::runtime_error(errMsg);
    };

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
};

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

uint32_t findMemoryType(const vk::raii::PhysicalDevice &physicalDevice,
                        uint32_t typeFilter,
                        vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memoryProperties =
        physicalDevice.getMemoryProperties();

    bool hasValidType;
    bool hasValidProperties;
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        hasValidType       = typeFilter & (1 << i);
        hasValidProperties = (memoryProperties.memoryTypes[i].propertyFlags &
                              properties) == properties;

        if (hasValidType && hasValidProperties)
            return i;
    };

    FTL_CRITICAL(
        "Failed to find valid memory type for vertex buffer allocation!");
    throw std::runtime_error("Failed to find valid memory type!");
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
    vk::KHRSwapchainExtensionName,
    vk::KHRSpirv14ExtensionName,
    vk::KHRSynchronization2ExtensionName,
    vk::KHRCreateRenderpass2ExtensionName,
};

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    { {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

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
    GTFO_PROFILE_FUNCTION();
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
    GTFO_PROFILE_FUNCTION();
    std::vector<vk::raii::PhysicalDevice> devices =
        mInstance.enumeratePhysicalDevices();

    if (devices.empty())
        throw std::runtime_error("Failed to find Vulkan Supported GPUs!");
    {
        GTFO_PROFILE_SCOPE("Vulkan: Pick Physical Device", "func");
        bool hasVulkan14Support;
        bool hasRequiredProperties;
        bool hasRequiredFeatures;

        vk::raii::PhysicalDevice device {nullptr};
        for (uint32_t i = 0; i < devices.size(); i++) {
            device                = devices[i];
            auto deviceProperties = device.getProperties();
            auto deviceFeatures   = device.getFeatures();
            bool isValidDevice =
                deviceProperties.apiVersion >= vk::ApiVersion14;

            auto qfps = device.getQueueFamilyProperties();
            auto graphicsQfpPredicate =
                [](vk::QueueFamilyProperties const &qfp) {
                    return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) !=
                           static_cast<vk::QueueFlags>(0);
                };

            const auto qfpIter =
                std::ranges::find_if(qfps, graphicsQfpPredicate);
            isValidDevice         = isValidDevice && (qfpIter != qfps.end());

            auto deviceExtensions = device.enumerateDeviceExtensionProperties();
            for (const char *const &deviceExtension :
                 RequiredDeviceExtensions) {
                auto predicate = [deviceExtension](auto const &ext) {
                    return strcmp(ext.extensionName, deviceExtension) == 0;
                };

                auto extIter =
                    std::ranges::find_if(deviceExtensions, predicate);
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
    }

    FTL_CRITICAL("No valid physical devices were found!");
    throw std::runtime_error("No valid physical devices were found!");
};

void Renderer::createLogicalDevice() {
    GTFO_PROFILE_FUNCTION();
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
    mGraphicsQueueIndex    = graphicsIndex;

    uint32_t presentIndex =
        mPhysicalDevice.getSurfaceSupportKHR(graphicsIndex, mSurface)
            ? graphicsIndex
            : static_cast<uint32_t>(queueFamilyProperties.size());
    mPresentQueueIndex = presentIndex;

    if (presentIndex == queueFamilyProperties.size()) {
        FTL_DEBUG("Finding Present Queue Index...");
        bool hasGraphicsSupport = true;
        bool hasPresentSupport  = true;
        {
            GTFO_PROFILE_SCOPE("Vulkan: Finding Present Queue Index", "func");
            for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
                hasGraphicsSupport =
                    static_cast<bool>(queueFamilyProperties[i].queueFlags &
                                      vk::QueueFlagBits::eGraphics);
                hasGraphicsSupport = mPhysicalDevice.getSurfaceSupportKHR(
                    static_cast<uint32_t>(i), mSurface);

                if (hasGraphicsSupport && hasPresentSupport) {
                    graphicsIndex       = static_cast<uint32_t>(i);
                    mGraphicsQueueIndex = graphicsIndex;

                    presentIndex        = graphicsIndex;
                    mPresentQueueIndex  = presentIndex;
                    FTL_DEBUG("Graphics Queue and Present Queue have the same "
                              "indicies: {}, {}  respectively",
                              graphicsIndex, presentIndex);
                    break;
                } else if (!hasGraphicsSupport && hasPresentSupport) {
                    presentIndex       = static_cast<uint32_t>(i);
                    mPresentQueueIndex = presentIndex;
                    FTL_DEBUG("Graphics Queue and Present Queue have different "
                              "indicies: {}, {}  respectively",
                              graphicsIndex, presentIndex);
                    break;
                }
            };
        }

        if (graphicsIndex == queueFamilyProperties.size() ||
            presentIndex == queueFamilyProperties.size()) {
            constexpr const char *errMsg = "No valid Vulkan Queue was found "
                                           "that supports graphics or present!";
            FTL_CRITICAL(errMsg);
            throw std::runtime_error(errMsg);
        };
    };

    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan11Features,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
                       vk::PhysicalDeviceSwapchainMaintenance1FeaturesEXT>
        featureChain {
            {}, // vk::PhysicalDeviceFeatures2 (empty for now)
            {.shaderDrawParameters = VK_TRUE},
            {.synchronization2 = VK_TRUE,
             .dynamicRendering =
                 VK_TRUE}, // Enable dynamic rendering from Vulkan 1.3
            {.extendedDynamicState =
                 VK_TRUE}, // Enable extended dynamic state from the extension
            {.swapchainMaintenance1 = VK_TRUE},
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
    GTFO_PROFILE_FUNCTION();
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
    GTFO_PROFILE_FUNCTION();

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

void Renderer::createGraphicsPipeline() {
    GTFO_PROFILE_FUNCTION();
    std::string shaderPath       = std::string(std::filesystem::current_path());
    shaderPath                   = shaderPath + "/assets/shaders/slang.spv";

    std::vector<char> shaderCode = readFile(shaderPath);
    vk::ShaderModuleCreateInfo shaderCreateInfo {
        .codeSize = shaderCode.size() * sizeof(char),
        .pCode    = reinterpret_cast<const uint32_t *>(shaderCode.data())};

    vk::raii::ShaderModule shaderModule {mDevice, shaderCreateInfo};

    vk::PipelineShaderStageCreateInfo vertShaderCreateInfo {
        .stage  = vk::ShaderStageFlagBits::eVertex,
        .module = shaderModule,
        .pName  = "vertMain"};

    vk::PipelineShaderStageCreateInfo fragShaderCreateInfo {
        .stage  = vk::ShaderStageFlagBits::eFragment,
        .module = shaderModule,
        .pName  = "fragMain"};

    vk::PipelineShaderStageCreateInfo shaderStages[] {vertShaderCreateInfo,
                                                      fragShaderCreateInfo};

    auto bindingDescription    = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions    = &bindingDescription,
        .vertexAttributeDescriptionCount =
            static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()};

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly {
        .topology = vk::PrimitiveTopology::eTriangleList};

    vk::PipelineViewportStateCreateInfo viewportState {.viewportCount = 1,
                                                       .scissorCount  = 1};

    vk::PipelineRasterizationStateCreateInfo rasterizer {
        .depthClampEnable        = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode             = vk::PolygonMode::eFill,
        .cullMode                = vk::CullModeFlagBits::eBack,
        .frontFace               = vk::FrontFace::eClockwise,
        .depthBiasEnable         = vk::False,
        .depthBiasSlopeFactor    = 1.0f,
        .lineWidth               = 1.0f};

    vk::PipelineMultisampleStateCreateInfo multisampling {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable  = vk::False};

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = vk::False;

    vk::PipelineColorBlendStateCreateInfo colorBlending {
        .logicOpEnable   = vk::False, //
        .logicOp         = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments    = &colorBlendAttachment};

    std::vector<vk::DynamicState> dynamicStates {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
    };

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo {
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates    = dynamicStates.data()};

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo {
        .setLayoutCount = 0, .pushConstantRangeCount = 0};

    mPipelineLayout =
        vk::raii::PipelineLayout(mDevice, pipelineLayoutCreateInfo);

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo {
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &mSwapChainFormat};

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo {
        .pNext               = &pipelineRenderingCreateInfo,
        .stageCount          = 2,
        .pStages             = shaderStages,
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicStateCreateInfo,
        .layout              = mPipelineLayout,
        .renderPass          = nullptr};

    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex  = -1;

    mGraphicsPipeline =
        vk::raii::Pipeline(mDevice, nullptr, pipelineCreateInfo);

    FTL_DEBUG("Vulkan Graphics Pipeline Successfully Created!");
};

void Renderer::createCommandPool() {
    vk::CommandPoolCreateInfo createInfo {
        .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = mGraphicsQueueIndex};

    mCommandPool = vk::raii::CommandPool(mDevice, createInfo);
};

void Renderer::createVertexBuffer() {
    vk::BufferCreateInfo bufferCreateInfo {
        .size        = sizeof(vertices[0]) * vertices.size(),
        .usage       = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive};

    mVertexBuffer = vk::raii::Buffer(mDevice, bufferCreateInfo);

    vk::MemoryRequirements memoryRequirements =
        mVertexBuffer.getMemoryRequirements();

    vk::MemoryAllocateInfo memoryAllocateInfo {
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex =
            findMemoryType(mPhysicalDevice, memoryRequirements.memoryTypeBits,
                           vk::MemoryPropertyFlagBits::eHostVisible |
                               vk::MemoryPropertyFlagBits::eHostCoherent)};

    mVertexBufferMemory = vk::raii::DeviceMemory(mDevice, memoryAllocateInfo);
    mVertexBuffer.bindMemory(*mVertexBufferMemory, 0);

    void *data = mVertexBufferMemory.mapMemory(0, bufferCreateInfo.size);
    memcpy(data, vertices.data(), bufferCreateInfo.size);
    mVertexBufferMemory.unmapMemory();
};

void Renderer::createCommandBuffer() {
    vk::CommandBufferAllocateInfo allocInfo {
        .commandPool        = mCommandPool,
        .level              = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1};

    mCommandBuffer =
        std::move(vk::raii::CommandBuffers(mDevice, allocInfo).front());
};

void Renderer::recordCommandBuffer(uint32_t imageIndex) {
    mCommandBuffer.begin({});

    transitionImageLayout(
        imageIndex, vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {}, // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite,        // dstAccessMask
        vk::PipelineStageFlagBits2::eTopOfPipe,            // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput // dstStage
    );

    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo {
        .imageView   = mSwapChainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp      = vk::AttachmentLoadOp::eClear,
        .storeOp     = vk::AttachmentStoreOp::eStore,
        .clearValue  = clearColor};

    vk::RenderingInfo renderingInfo {
        .renderArea           = {.offset = {0, 0}, .extent = mSwapChainExtent},
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &attachmentInfo
    };

    mCommandBuffer.beginRendering(renderingInfo);
    mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                mGraphicsPipeline);

    mCommandBuffer.bindVertexBuffers(0, *mVertexBuffer, {0});

    mCommandBuffer.setViewport(
        0,
        vk::Viewport(0.0f, 0.0f, static_cast<float>(mSwapChainExtent.width),
                     static_cast<float>(mSwapChainExtent.height), 0.0f, 1.0f));

    mCommandBuffer.setScissor(0,
                              vk::Rect2D(vk::Offset2D(0, 0), mSwapChainExtent));
    mCommandBuffer.draw(3, 1, 0, 0);
    mCommandBuffer.endRendering();

    transitionImageLayout(
        imageIndex, vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,         // srcAccessMask
        {},                                                 // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe           // dstStage
    );

    mCommandBuffer.end();
};

void Renderer::createSyncObjects() {
    mSemaphorePresentComplete =
        vk::raii::Semaphore(mDevice, vk::SemaphoreCreateInfo());

    mSemaphoreRenderFinished =
        vk::raii::Semaphore(mDevice, vk::SemaphoreCreateInfo());

    mFenceDraw =
        vk::raii::Fence(mDevice, {.flags = vk::FenceCreateFlagBits::eSignaled});
};

void Renderer::render() {
    GTFO_PROFILE_FUNCTION();
    auto [result, imageIndex] = mSwapChain.acquireNextImage(
        UINT64_MAX, *mSemaphorePresentComplete, nullptr);

    recordCommandBuffer(imageIndex);

    mDevice.resetFences(*mFenceDraw);

    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput);

    const vk::SubmitInfo submitInfo {
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &*mSemaphorePresentComplete,
        .pWaitDstStageMask    = &waitDestinationStageMask,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &*mCommandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &*mSemaphoreRenderFinished};

    mGraphicsQueue.submit(submitInfo, *mFenceDraw);
    while (vk::Result::eTimeout ==
           mDevice.waitForFences(*mFenceDraw, vk::True, UINT64_MAX))
        ;

    const vk::PresentInfoKHR presentInfoKHR {.waitSemaphoreCount = 1,
                                             .pWaitSemaphores =
                                                 &*mSemaphoreRenderFinished,
                                             .swapchainCount = 1,
                                             .pSwapchains    = &*mSwapChain,
                                             .pImageIndices  = &imageIndex};

    result = mGraphicsQueue.presentKHR(presentInfoKHR);
};

void Renderer::transitionImageLayout(uint32_t imageIndex,
                                     vk::ImageLayout oldLayout,
                                     vk::ImageLayout newLayout,
                                     vk::AccessFlags2 srcAccessMask,
                                     vk::AccessFlags2 dstAccessMask,
                                     vk::PipelineStageFlags2 srcStageMask,
                                     vk::PipelineStageFlags2 dstStageMask) {
    GTFO_PROFILE_FUNCTION();
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask        = srcStageMask,
        .srcAccessMask       = srcAccessMask,
        .dstStageMask        = dstStageMask,
        .dstAccessMask       = dstAccessMask,
        .oldLayout           = oldLayout,
        .newLayout           = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = mSwapChainImages[imageIndex],
        .subresourceRange    = {.aspectMask     = vk::ImageAspectFlagBits::eColor,
                                .baseMipLevel   = 0,
                                .levelCount     = 1,
                                .baseArrayLayer = 0,
                                .layerCount     = 1}
    };
    vk::DependencyInfo dependencyInfo = {.dependencyFlags         = {},
                                         .imageMemoryBarrierCount = 1,
                                         .pImageMemoryBarriers    = &barrier};
    mCommandBuffer.pipelineBarrier2(dependencyInfo);
}

void Renderer::shutdown(GLFWwindow **ppWindow) {
    glfwDestroyWindow(*ppWindow);
    glfwTerminate();
};
}; // namespace FTL

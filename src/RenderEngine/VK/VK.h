#pragma once
#include "../glfw_vulkan.h"

#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>

#include "../../using_std.h"

#include "VK_DFL.h"

#include "../../Logging.h"

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;

    bool isComplete() {
        return graphics.has_value() && present.has_value();
    }

    set<uint32_t> unique_set() {
        return set<uint32_t>{graphics.value(), present.value()};
    }
};

struct SurfaceDetails {
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilities{};
    vector<VkSurfaceFormatKHR> vkSurfaceFormats;
    vector<VkPresentModeKHR> vkPresentModes;
};


namespace VK {
    VkDebugUtilsMessengerEXT vkDebugUtilsMessengerEXT;

    inline bool supportsLayers(vector<const char*> layers);


    VkBool32 debugCallbackDefault(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                  void *pUserData) {
        if (messageSeverity > 1) std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    VkBool32 (*debugCallback)(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) = debugCallbackDefault;

    VkDebugUtilsMessengerCreateInfoEXT vkDefaultDebugUtilsMessengerCreateInfoEXT = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
    };

    inline bool createDebugMessenger(VkInstance vkInstance,
                                     VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfoEXT = vkDefaultDebugUtilsMessengerCreateInfoEXT) {
        return CreateDebugUtilsMessengerEXT(vkInstance, &vkDebugUtilsMessengerCreateInfoEXT, nullptr, &vkDebugUtilsMessengerEXT);
    }

    inline void deleteDebugMessenger(VkInstance vkInstance) {
        DestroyDebugUtilsMessengerEXT(vkInstance, vkDebugUtilsMessengerEXT, nullptr);
    }

    inline VkInstance createInstance(vector<const char*> extensions = { },
                                     vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" },
                                     const char* appName = "",
                                     VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfoEXT = vkDefaultDebugUtilsMessengerCreateInfoEXT) {

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

        VkInstanceCreateInfo vkInstanceCreateInfo{};
        vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        VkApplicationInfo vkAppInfo{};
        vkInstanceCreateInfo.pApplicationInfo = &vkAppInfo;
        vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        vkAppInfo.pApplicationName = appName;
        vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        vkAppInfo.pEngineName = "";
        vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        vkAppInfo.apiVersion = VK_API_VERSION_1_0;

        vkInstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        vkInstanceCreateInfo.ppEnabledExtensionNames = extensions.data();

        if (!layers.empty()) {
            if (!supportsLayers(layers)) {
                throw std::runtime_error("requested layers not available!");
            }

            vkInstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
            vkInstanceCreateInfo.ppEnabledLayerNames = layers.data();

            vkInstanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &vkDebugUtilsMessengerCreateInfoEXT;
        } else {
            vkInstanceCreateInfo.enabledLayerCount = 0;
        }

        VkInstance vkInstance;
        vkCreateInstance(&vkInstanceCreateInfo, nullptr, &vkInstance);

        return vkInstance;
    }

    inline void deleteInstance(VkInstance vkInstance) {
        vkDestroyInstance(vkInstance, nullptr);
    }


    inline VkSurfaceKHR createSurface(VkInstance vkInstance, GLFWwindow* window) {
        VkWin32SurfaceCreateInfoKHR vkCreateInfo = {};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        vkCreateInfo.hwnd = glfwGetWin32Window(window);
        vkCreateInfo.hinstance = GetModuleHandle(nullptr);

        /*if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) { throw std::runtime_error("failed to create window surface!"); }*/
        VkSurfaceKHR vkSurface;
        vkCreateWin32SurfaceKHR(vkInstance, &vkCreateInfo, nullptr, &vkSurface);

        return vkSurface;
    }

    inline void deleteSurface(VkInstance vkInstance, VkSurfaceKHR vkSurface) {
        vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
    }

    inline vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance vkInstance) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        if (deviceCount != 0) {
            vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
        }

        return devices;
    }

    inline VkDeviceQueueCreateInfo getQueueCreateInfo(uint32_t queueFamilyIndex,
                                                      uint32_t queueCount,
                                                      float &queuePriority) {
        VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo{};
        vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        vkDeviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        vkDeviceQueueCreateInfo.queueCount = queueCount;
        vkDeviceQueueCreateInfo.pQueuePriorities = &queuePriority;
        return vkDeviceQueueCreateInfo;
    }

    inline vector<VkDeviceQueueCreateInfo> getQueueCreateInfos(QueueFamilyIndices indices) {
        vector<VkDeviceQueueCreateInfo> vkDeviceQueueCreateInfos{};

        float queuePriority = 0.5f;
        for (uint32_t queueFamilyIndex : indices.unique_set()) {
            vkDeviceQueueCreateInfos.push_back(getQueueCreateInfo(queueFamilyIndex, 1, queuePriority));
        }

        return vkDeviceQueueCreateInfos;
    }

    inline VkDevice createLogicalDevice(VkPhysicalDevice vkPhysicalDevice,
                                        vector<VkDeviceQueueCreateInfo> queueCreateInfos,
                                        vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME },
                                        VkPhysicalDeviceFeatures* features = nullptr,
                                        vector <const char*> validationLayers = vector<const char*>(0)) {
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = features;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (!validationLayers.empty()) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        VkDevice vkDevice;
        vkCreateDevice(vkPhysicalDevice, &createInfo, nullptr, &vkDevice);

        return vkDevice;
    }

    inline void deleteLogicalDevice(VkDevice vkDevice) {
        vkDestroyDevice(vkDevice, nullptr);
    }

    inline  VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(VkPhysicalDevice vkPhysicalDevice) {
        VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vkPhysicalDeviceFeatures);
        return vkPhysicalDeviceFeatures;
    }

    inline  VkPhysicalDeviceProperties getPhysicalDeviceProperties(VkPhysicalDevice vkPhysicalDevice) {
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkPhysicalDeviceProperties);
        return vkPhysicalDeviceProperties;
    }

    inline vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice vkPhysicalDevice) {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, nullptr);
        vector<VkQueueFamilyProperties> queueFamilyProperties(count);
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, queueFamilyProperties.data());
        return queueFamilyProperties;
    }

    // select the first queue that has present & graphics abilities
    inline QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface,
                                                    vector<VkQueueFamilyProperties> vkQueueFamilyProperties = vector<VkQueueFamilyProperties>{0}) {
        QueueFamilyIndices indices;

        if (vkQueueFamilyProperties.empty()) {
            vkQueueFamilyProperties = getQueueFamilyProperties(vkPhysicalDevice);
        }

        int i = 0;
        for (const auto& queueFamily : vkQueueFamilyProperties) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentSupport);

            if (presentSupport) indices.present = i;
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphics = i;
            if (indices.isComplete()) break;
            i++;
        }

        return indices;
    }

    inline VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkPhysicalDevice vkDevice, VkSurfaceKHR vkSurface) {
        VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkDevice, vkSurface, &vkSurfaceCapabilitiesKHR);
        return vkSurfaceCapabilitiesKHR;
    }

    inline vector<VkSurfaceFormatKHR> enumerateSurfaceFormats(VkPhysicalDevice vkDevice, VkSurfaceKHR vkSurface) {
        uint32_t count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice, vkSurface, &count, nullptr);
        vector<VkSurfaceFormatKHR> surfaceFormats(count);
        if (count != 0) vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice, vkSurface, &count, surfaceFormats.data());
        return surfaceFormats;
    }

    inline vector<VkPresentModeKHR> enumeratePresentModes(VkPhysicalDevice vkDevice, VkSurfaceKHR vkSurface) {
        uint32_t count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkDevice, vkSurface, &count, nullptr);
        vector<VkPresentModeKHR> presentModes(count);
        if (count != 0) vkGetPhysicalDeviceSurfacePresentModesKHR(vkDevice, vkSurface, &count, presentModes.data());

        return presentModes;
    }

    inline SurfaceDetails getSurfaceDetails(VkPhysicalDevice vkDevice, VkSurfaceKHR vkSurface) {
        SurfaceDetails details;

        details.vkSurfaceCapabilities = getSurfaceCapabilities(vkDevice, vkSurface);
        details.vkSurfaceFormats = enumerateSurfaceFormats(vkDevice, vkSurface);
        details.vkPresentModes = enumeratePresentModes(vkDevice, vkSurface);

        return details;
    }

    inline bool supportsLayers(vector<const char*> layers) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : layers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    inline bool supportsExtensions(VkPhysicalDevice vkPhysicalDevice, vector<const char*> extensions) {
        uint32_t count;
        vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &count, nullptr);

        vector<VkExtensionProperties> availableExtensions(count);
        vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &count, availableExtensions.data());

        set<std::string> requiredExtensions(extensions.begin(), extensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    // ********* //
    // UTILITIES //
    // ********* //

    inline VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats,
                                                          VkFormat format = VK_FORMAT_B8G8R8A8_SRGB,
                                                          VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == format && availableFormat.colorSpace == colorSpace) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    inline VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes,
                                                      VkPresentModeKHR mode = VK_PRESENT_MODE_MAILBOX_KHR) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == mode) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    inline VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D actualExtent = { width, height };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    inline VkSwapchainKHR createSwapchain(VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, VkSurfaceKHR vkSurface, uint32_t width, uint32_t height) {
        SurfaceDetails surfaceDetails = getSurfaceDetails(vkPhysicalDevice, vkSurface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(surfaceDetails.vkSurfaceFormats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(surfaceDetails.vkPresentModes);
        VkExtent2D extent = chooseSwapExtent(surfaceDetails.vkSurfaceCapabilities, width, height);

        uint32_t imageCount = surfaceDetails.vkSurfaceCapabilities.minImageCount + 1;
        if (surfaceDetails.vkSurfaceCapabilities.maxImageCount > 0
        && imageCount > surfaceDetails.vkSurfaceCapabilities.maxImageCount) {
            imageCount = surfaceDetails.vkSurfaceCapabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR{};
        vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        vkSwapchainCreateInfoKHR.surface = vkSurface;

        vkSwapchainCreateInfoKHR.minImageCount = imageCount;
        vkSwapchainCreateInfoKHR.imageFormat = surfaceFormat.format;
        vkSwapchainCreateInfoKHR.imageColorSpace = surfaceFormat.colorSpace;
        vkSwapchainCreateInfoKHR.imageExtent = extent;
        vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
        vkSwapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = getQueueFamilyIndices(vkPhysicalDevice, vkSurface, getQueueFamilyProperties(vkPhysicalDevice));
        uint32_t queueFamilyIndices[] = {indices.graphics.value(), indices.present.value()};

        if (indices.graphics != indices.present) {
            vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            vkSwapchainCreateInfoKHR.queueFamilyIndexCount = 2;
            vkSwapchainCreateInfoKHR.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        vkSwapchainCreateInfoKHR.preTransform = surfaceDetails.vkSurfaceCapabilities.currentTransform;
        vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        vkSwapchainCreateInfoKHR.presentMode = presentMode;
        vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

        VkSwapchainKHR vkSwapchainKHR;
        vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, nullptr, &vkSwapchainKHR);

        return vkSwapchainKHR;
    }

    inline VkImageView createImageView(VkDevice vkDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        vkCreateImageView(vkDevice, &viewInfo, nullptr, &imageView);
        return imageView;
    }

    inline VkCommandPool createCommandPool(VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, VkSurfaceKHR vkSurface) {
        QueueFamilyIndices queueFamilyIndices = getQueueFamilyIndices(vkPhysicalDevice, vkSurface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphics.value();

        VkCommandPool  vkCommandPool;
        if (vkCreateCommandPool(vkDevice, &poolInfo, nullptr, &vkCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }

    void createGraphicsPipeline() {
        auto vertShaderCode = readFile("shaders/vert.spv");
        auto fragShaderCode = readFile("shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapChainExtent.width;
        viewport.height = (float) swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &vkDescriptorSetLayout;

        if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = vkPipelineLayout;
        pipelineInfo.renderPass = vkRenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkGraphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);
    }

    inline VkFramebuffer createFramebuffer(VkDevice vkDevice, VkRenderPass vkRenderPass, VkSwapchainKHR vkSwapchain, uint32_t width, uint32_t height) {
        std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vkRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

        VkFramebuffer vkFrameBuffer;
        vkCreateFramebuffer(vkDevice, &framebufferInfo, nullptr, &vkFrameBuffer);
    }

    inline bool isDeviceSuitable(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface) {
        QueueFamilyIndices indices = getQueueFamilyIndices(vkPhysicalDevice, vkSurface,
                                                           getQueueFamilyProperties(vkPhysicalDevice));

        bool extensionsSupported = supportsExtensions(vkPhysicalDevice, { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SurfaceDetails surfaceDetails = getSurfaceDetails(vkPhysicalDevice, vkSurface);
            swapChainAdequate = !surfaceDetails.vkSurfaceFormats.empty() && !surfaceDetails.vkPresentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    inline uint32_t getPhysicalDeviceScore(VkPhysicalDevice vkPhysicalDevice) {
        uint32_t score = 0;

        VkPhysicalDeviceProperties properties = getPhysicalDeviceProperties(vkPhysicalDevice);
        VkPhysicalDeviceFeatures features = getPhysicalDeviceFeatures(vkPhysicalDevice);

        if (!features.geometryShader) return 0;
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

        score += properties.limits.maxImageDimension2D;

        return score;
    }

    inline VkPhysicalDevice getBestPhysicalDevice(VkInstance vkInstance, VkSurfaceKHR vkSurface) {
        vector<VkPhysicalDevice> devices = enumeratePhysicalDevices(vkInstance);
        if (devices.empty()) return nullptr;

        multimap<int, VkPhysicalDevice> candidates;

        for (const auto& device : devices) {
            if (isDeviceSuitable(device, vkSurface)) {
                candidates.insert(make_pair(getPhysicalDeviceScore(device), device));
            }
        }

        return candidates.rbegin()->second;
    }
};
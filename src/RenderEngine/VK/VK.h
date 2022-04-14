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
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
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

    // "VK_KHR_SWAPCHAIN_EXTENSION_NAME"
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

    inline VkDevice createLogicalDevice(VkPhysicalDevice vkPhysicalDevice,
                                            vector<const char*> extensions,
                                            vector<VkDeviceQueueCreateInfo> queueCreateInfos,
                                            VkPhysicalDeviceFeatures* features = nullptr,
                                            vector <const char*> validationLayers = vector<const char*>(0)) {
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = features;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (validationLayers.size() != 0) {
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

    inline vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice vkPhysicalDevice) {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, nullptr);
        vector<VkQueueFamilyProperties> queueFamilyProperties(count);
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, queueFamilyProperties.data());
        return queueFamilyProperties;
    }

    // select the first queue that has present & graphics abilities
    inline QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface, vector<VkQueueFamilyProperties> vkQueueFamilyProperties) {
        QueueFamilyIndices indices;

        int i = 0;
        for (const auto& queueFamily : vkQueueFamilyProperties) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentSupport);

            if (presentSupport) indices.presentFamily = i;
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
            if (indices.isComplete()) break;
            i++;
        }

        return indices;
    }

    inline SurfaceDetails getSurfaceDetails(VkPhysicalDevice device, VkSurfaceKHR vkSurface) {
        SurfaceDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkSurface, &details.vkSurfaceCapabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.vkSurfaceFormats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, details.vkSurfaceFormats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.vkPresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, details.vkPresentModes.data());
        }

        return details;
    }

    inline bool supportsLayers(vector<const char*> layers) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
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
            int width, height;

            VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

//    inline void createSwapChain(VkDevice vkPhysicalDevice, vkSurface) {
//        SurfaceDetails surfaceDetails = getSurfaceDetails(vkPhysicalDevice, vkSurface);
//    }
};
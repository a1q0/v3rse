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

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> vkSurfaceFormats;
    std::vector<VkPresentModeKHR> vkPresentModes;
};

namespace VK {
    VkDebugUtilsMessengerCreateInfoEXT vkDefaultDebugUtilsMessengerCreateInfoEXT{};

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackDefault(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    static VKAPI_ATTR VkBool32 VKAPI_CALL (*debugCallback)(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) = debugCallbackDefault;

    inline vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice vkPhysicalDevice);
    inline QueueFamilyIndices getQueueFamilyIndices(vector<VkQueueFamilyProperties> vkQueueFamilyProperties);

    VkInstance createInstance(vector<const char *> extensions, vector<const char *> layers, const char *appName,
                              VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfoEXT);
    inline void deleteInstance(VkInstance);

    inline VkSurfaceKHR createSurface(VkInstance vkInstance, GLFWwindow *window);
    inline void deleteSurface(VkInstance vkInstance, VkSurfaceKHR vkSurface);

    vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance vkInstance);

    inline VkDevice createLogicalDevice(VkPhysicalDevice vkPhysicalDevice,
                                        vector<const char*> extensions,
                                        vector<VkDeviceQueueCreateInfo> queueCreateInfos,
                                        VkPhysicalDeviceFeatures* features, // optional
                                        vector <const char*> validationLayers); // optional
    inline void deleteLogicalDevice(VkDevice device);

    bool supportsLayers(vector<const char *> layers);
};
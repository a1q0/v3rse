#pragma once

//#include "../glfw_vulkan.h"
#include "glfw_vulkan.h"

namespace VK {
    _inline VkPhysicalDeviceMemoryProperties
    getPhysicalDeviceMemoryProperties(VkPhysicalDevice
    vkPhysicalDevice) {
    VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, & vkPhysicalDeviceMemoryProperties
    );

    return
    vkPhysicalDeviceMemoryProperties;
}
}
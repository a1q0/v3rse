#pragma once

#include "glfw_vulkan.h"
#include "../../Logging.h"

namespace VK {
    _inline VkPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties(VkPhysicalDevice vkPhysicalDevice) {
        VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vkPhysicalDeviceMemoryProperties);

        for (int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryHeapCount; i++) {
            info("{} -> {}", i, vkPhysicalDeviceMemoryProperties.memoryHeaps->size);
        }

        return vkPhysicalDeviceMemoryProperties;
    }
}
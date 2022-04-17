#pragma once

#include "glfw_vulkan.h"
#include "logging.h"

namespace VK {
    _inline VkMemoryProperties getPhysicalDeviceMemoryProperties(VkPhysicalDevice vkPhysicalDevice) {
        VkMemoryProperties vkMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vkMemoryProperties);

        for (int i = 0; i < vkMemoryProperties.memoryHeapCount; i++) {
            info("{} -> {} bytes", i, vkMemoryProperties.memoryHeaps->size);
        }

        for (int i = 0; i < vkMemoryProperties.memoryTypeCount; i++) {
            //info("{} -> {} heap index", i, vkPhysicalDeviceMemoryProperties.memoryTypes->heapIndex);
            info("{} -> {} heap index", i, vkMemoryProperties.memoryTypes->propertyFlags);
        }


        return vkMemoryProperties;
    }

    _inline uint32_t findMemoryType(VkPhysicalDevice vkPhysicalDevice) {
        VkMemoryProperties vkMemoryProperties = getPhysicalDeviceMemoryProperties(vkPhysicalDevice);

        for (int i = 0; i < vkMemoryProperties.memoryTypeCount; i++) {

        }

        return 0;
    }
}
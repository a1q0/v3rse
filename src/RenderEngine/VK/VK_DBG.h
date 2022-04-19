#pragma once

#include "glfw_vulkan.h"

namespace VK {
    force_inline void CHECK(VkResult result, const char* msg = "Vulkan check failed!") {
        if (result != VK_SUCCESS) throw std::runtime_error(msg);
    }

    VkDebugUtilsMessengerEXT vkDebugUtilsMessengerEXT;

    force_inline bool supportsLayers(vector<const char*> layers);


    VkBool32 debugCallbackDefault(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                  void* pUserData) {
        if (messageSeverity >= 0) std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    VkBool32 (* debugCallback)(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                               VkDebugUtilsMessageTypeFlagsEXT messageType,
                               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                               void* pUserData) = debugCallbackDefault;

    VkDebugUtilsMessengerCreateInfoEXT vkDefaultDebugUtilsMessengerCreateInfoEXT = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };

    force_inline bool createDebugMessenger(VkInstance vkInstance,
                                           VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfoEXT = vkDefaultDebugUtilsMessengerCreateInfoEXT) {
        return CreateDebugUtilsMessengerEXT(vkInstance, &vkDebugUtilsMessengerCreateInfoEXT, nullptr,
                                            &vkDebugUtilsMessengerEXT);
    }

    force_inline bool createDebugMessenger(
        VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfoEXT = vkDefaultDebugUtilsMessengerCreateInfoEXT) {
        return CreateDebugUtilsMessengerEXT(VK::instance, &vkDebugUtilsMessengerCreateInfoEXT, nullptr,
                                            &vkDebugUtilsMessengerEXT);
    }

    force_inline void deleteDebugMessenger(VkInstance vkInstance) {
        DestroyDebugUtilsMessengerEXT(vkInstance, vkDebugUtilsMessengerEXT, nullptr);
    }
}

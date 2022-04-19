#pragma once

#include "glfw_vulkan.h"
#include "using_std.h"
#include "VK_GLOBALS.h"
#include "VK_DBG.h"

namespace VK {
    force_inline VkInstance createInstance(vector<const char*> extensions = {},
                                           vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"},
                                           const char* appName = "",
                                           VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfoEXT = vkDefaultDebugUtilsMessengerCreateInfoEXT) {

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

        VkInstanceCreateInfo vkInstanceCreateInfo {};
        vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        VkApplicationInfo vkAppInfo {};
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

        vkCreateInstance(&vkInstanceCreateInfo, nullptr, &VK::instance);
        return VK::instance;
    }

    force_inline void deleteInstance(VkInstance vkInstance) {
        vkDestroyInstance(vkInstance, nullptr);
    }

    force_inline void deleteInstance() {
        deleteInstance(VK::instance);
    }

    force_inline VkSurfaceKHR createSurface(VkInstance vkInstance, GLFWwindow* window) {
        VkWin32SurfaceCreateInfoKHR vkCreateInfo = {};
        vkCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        vkCreateInfo.hwnd = glfwGetWin32Window(window);
        vkCreateInfo.hinstance = GetModuleHandle(nullptr);

        /*if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) { throw std::runtime_error("failed to create window surface!"); }*/
        VkSurfaceKHR vkSurface;
        vkCreateWin32SurfaceKHR(vkInstance, &vkCreateInfo, nullptr, &vkSurface);

        return vkSurface;
    }

    force_inline VkSurfaceKHR createSurface() {
        return createSurface(VK::instance, VK::window);
    }

    force_inline void deleteSurface(VkInstance vkInstance, VkSurfaceKHR vkSurface) {
        vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
    }

    force_inline void deleteSurface() {
        deleteSurface(VK::instance, VK::surface);
    }

    force_inline VkDevice createLogicalDevice(VkPhysicalDevice vkPhysicalDevice,
                                              vector<VkDeviceQueueCreateInfo> queueCreateInfos,
                                              vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                                              VkPhysicalDeviceFeatures* features = nullptr,
                                              vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"}) {
        VkDeviceCreateInfo createInfo {};
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
        CHECK(vkCreateDevice(vkPhysicalDevice, &createInfo, nullptr, &vkDevice));

        return vkDevice;
    }

    force_inline VkDevice createLogicalDevice(vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                                              VkPhysicalDeviceFeatures* features = nullptr,
                                              vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"}) {
        createLogicalDevice(VK::physicalDevice, queues.getQueueCreateInfos(), extensions, features, validationLayers);
    }

    force_inline void deleteLogicalDevice(VkDevice vkDevice) {
        vkDestroyDevice(vkDevice, nullptr);
    }

    force_inline void deleteLogicalDevice() {
        deleteLogicalDevice(VK::device);
    }

    force_inline vector<VkLayerProperties> enumerateInstanceLayersProperties() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        return availableLayers;
    }

    force_inline bool supportsLayers(vector<const char*> layers) {
        vector<VkLayerProperties> availableLayers = enumerateInstanceLayersProperties();

        for (const char* layerName: layers) {
            bool layerFound = false;

            for (const auto& layerProperties: availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) return false;
        }

        return true;
    }

    force_inline bool supportsExtensions(VkPhysicalDevice vkPhysicalDevice, vector<const char*> extensions) {
        uint32_t count;
        vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &count, nullptr);

        vector<VkExtensionProperties> availableExtensions(count);
        vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &count, availableExtensions.data());

        set<std::string> requiredExtensions(extensions.begin(), extensions.end());

        for (const auto& extension: availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    force_inline bool isDeviceSuitable(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface) {
        bool extensionsSupported = supportsExtensions(vkPhysicalDevice, {VK_KHR_SWAPCHAIN_EXTENSION_NAME});
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            swapChainAdequate = !surface.vkSurfaceFormats.empty() && !surface.vkPresentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &supportedFeatures);

        return queues.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    force_inline vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance vkInstance) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        if (deviceCount != 0) {
            vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
        }

        return devices;
    }

    force_inline VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(VkPhysicalDevice vkPhysicalDevice) {
        VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vkPhysicalDeviceFeatures);
        return vkPhysicalDeviceFeatures;
    }

    force_inline VkPhysicalDeviceProperties getPhysicalDeviceProperties(VkPhysicalDevice vkPhysicalDevice) {
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkPhysicalDeviceProperties);
        return vkPhysicalDeviceProperties;
    }

    force_inline uint32_t getPhysicalDeviceScore(VkPhysicalDevice vkPhysicalDevice) {
        uint32_t score = 0;

        VkPhysicalDeviceProperties properties = getPhysicalDeviceProperties(vkPhysicalDevice);
        VkPhysicalDeviceFeatures features = getPhysicalDeviceFeatures(vkPhysicalDevice);

        if (!features.geometryShader) return 0;
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

        score += properties.limits.maxImageDimension2D;

        return score;
    }

    force_inline VkPhysicalDevice getBestPhysicalDevice(VkInstance vkInstance, VkSurfaceKHR vkSurface) {
        vector<VkPhysicalDevice> devices = enumeratePhysicalDevices(vkInstance);
        if (devices.empty()) return nullptr;

        multimap<int, VkPhysicalDevice> candidates;

        for (const auto& device: devices) {
            if (isDeviceSuitable(device, vkSurface)) {
                candidates.insert(make_pair(getPhysicalDeviceScore(device), device));
            }
        }

        return candidates.rbegin()->second;
    }

    force_inline VkPhysicalDevice getBestPhysicalDevice() {
        return getBestPhysicalDevice(VK::instance, VK::surface);
    }

    force_inline void init() {
        VK::createInstance();
        VK::createDebugMessenger();
        VK::createSurface();
        VK::getBestPhysicalDevice();
        VK::queues.getQueueFamilyIndices();
    }
}
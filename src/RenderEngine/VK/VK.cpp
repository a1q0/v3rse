#include "VK.h"

VkBool32 VK::debugCallbackDefault(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}


inline VkInstance VK::createInstance(vector<const char*> extensions, vector<const char*> layers = vector<const char*>(0),
                                     const char* appName = "",
                                     VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfoEXT = vkDefaultDebugUtilsMessengerCreateInfoEXT) {

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

    if (layers.size() != 0) {
        if (!supportsLayers(layers)) {
            throw std::runtime_error("validation layers requested, but not available!");
        }
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        vkInstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        vkInstanceCreateInfo.ppEnabledLayerNames = layers.data();

//        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
//        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//        createInfo.pfnUserCallback = debugCallback;
//        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        vkInstanceCreateInfo.enabledLayerCount = 0;
        vkInstanceCreateInfo.pNext = nullptr;
    }

    VkInstance vkInstance;
    vkCreateInstance(&vkInstanceCreateInfo, nullptr, &vkInstance);

    return vkInstance;
}

inline void VK::deleteInstance(VkInstance vkInstance) {
    vkDestroyInstance(vkInstance, nullptr);
}

inline VkSurfaceKHR VK::createSurface(VkInstance vkInstance, GLFWwindow* window) {
    VkWin32SurfaceCreateInfoKHR vkCreateInfo = {};
    vkCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkCreateInfo.hwnd = glfwGetWin32Window(window);
    vkCreateInfo.hinstance = GetModuleHandle(nullptr);

    /*if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) { throw std::runtime_error("failed to create window surface!"); }*/
    VkSurfaceKHR vkSurface;
    vkCreateWin32SurfaceKHR(vkInstance, &vkCreateInfo, nullptr, &vkSurface);

    return vkSurface;
}

inline void VK::deleteSurface(VkInstance vkInstance, VkSurfaceKHR vkSurface) {
    vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
}

inline vector<VkPhysicalDevice> VK::enumeratePhysicalDevices(VkInstance vkInstance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (deviceCount != 0) {
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
    }

    return devices;
}

inline VkDevice VK::createLogicalDevice(VkPhysicalDevice vkPhysicalDevice,
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

inline void VK::deleteLogicalDevice(VkDevice vkDevice) {
    vkDestroyDevice(vkDevice, nullptr);
}

vector<VkQueueFamilyProperties> VK::getQueueFamilyProperties(VkPhysicalDevice vkPhysicalDevice) {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, nullptr);
    vector<VkQueueFamilyProperties> queueFamilyProperties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, queueFamilyProperties.data());
    return queueFamilyProperties;
}

QueueFamilyIndices VK::getQueueFamilyIndices(vector<VkQueueFamilyProperties> vkQueueFamilyProperties) {
    QueueFamilyIndices indices;

    int i = 0;
    for (const auto& queueFamily : vkQueueFamilyProperties) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

bool VK::supportsLayers(vector<const char*> layers) {
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


#pragma once
#include "../glfw_vulkan.h"

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
    inline void CHECK(VkResult result, const char* msg = "Vulkan check failed!") {
        if (result != VK_SUCCESS) {
            throw std::runtime_error(msg);
        }
    }

    VkDebugUtilsMessengerEXT vkDebugUtilsMessengerEXT;

    inline bool supportsLayers(vector<const char*> layers);


    VkBool32 debugCallbackDefault(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                  void *pUserData) {
        if (messageSeverity >= 0) std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
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
                                        vector <const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" }) {
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
        CHECK(vkCreateDevice(vkPhysicalDevice, &createInfo, nullptr, &vkDevice));

        return vkDevice;
    }

    inline void deleteLogicalDevice(VkDevice vkDevice) {
        vkDestroyDevice(vkDevice, nullptr);
    }

    inline VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(VkPhysicalDevice vkPhysicalDevice) {
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

        if (vkQueueFamilyProperties.empty()) vkQueueFamilyProperties = getQueueFamilyProperties(vkPhysicalDevice);

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

    inline vector<VkLayerProperties> enumerateInstanceLayersProperties() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        return availableLayers;
    }

    inline bool supportsLayers(vector<const char*> layers) {
        vector<VkLayerProperties> availableLayers = enumerateInstanceLayersProperties();

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

    inline VkSwapchainKHR createSwapchain(VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, VkSurfaceKHR vkSurface, const SurfaceDetails& surfaceDetails,
                                          uint32_t width, uint32_t height, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkExtent2D extent) {
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
            vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // best perf !
        }

        vkSwapchainCreateInfoKHR.preTransform = surfaceDetails.vkSurfaceCapabilities.currentTransform;
        vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        vkSwapchainCreateInfoKHR.presentMode = presentMode;
        vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

        VkSwapchainKHR vkSwapchainKHR;
        vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, nullptr, &vkSwapchainKHR);

        return vkSwapchainKHR;
    }

    inline void deleteSwapchain(VkDevice vkDevice, VkSwapchainKHR vkSwapchain) {
        vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);
    }

    inline vector<VkImage> getSwapchainImages(VkDevice vkDevice, VkSwapchainKHR vkSwapchainKHR) {
        uint32_t count;
        vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &count, nullptr);
        vector<VkImage> vkImages(count);
        vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &count, vkImages.data());
        return vkImages;
    }

    inline VkImageView createImageView(VkDevice vkDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo vkImageViewCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                    .aspectMask = aspectFlags,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
            }
        };

        VkImageView imageView;
        vkCreateImageView(vkDevice, &vkImageViewCreateInfo, nullptr, &imageView);
        return imageView;
    }

    inline void deleteImageView(VkDevice vkDevice, VkImageView vkImageView) {
        vkDestroyImageView(vkDevice, vkImageView, nullptr);
    }

    inline void deleteImage(VkDevice vkDevice, VkImage vkImage) {
        vkDestroyImage(vkDevice, vkImage, nullptr);
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
        return vkCommandPool;
    }

    inline void deleteCommandPool(VkDevice vkDevice, VkCommandPool vkCommandPool) {
        vkDestroyCommandPool(vkDevice, vkCommandPool, nullptr);
    }

    inline void recordCommandBuffer(VkCommandBuffer vkCommandBuffer, uint32_t image_index, VkRenderPass vkRenderPass, VkFramebuffer vkFramebuffer, VkExtent2D extent, VkPipeline vkPipeline) {
        VkCommandBufferBeginInfo beginInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext{},
            .flags{},
            .pInheritanceInfo{}
        };
        vkBeginCommandBuffer(vkCommandBuffer, &beginInfo);

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        VkRenderPassBeginInfo renderPassInfo {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext{},
            .renderPass = vkRenderPass,
            .framebuffer = vkFramebuffer,
            .renderArea {
                    .offset = {0, 0},
                    .extent = extent
                },
            .clearValueCount = 1,
            .pClearValues = &clearColor
        };

        vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
        vkCmdDraw(vkCommandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(vkCommandBuffer);

        if (vkEndCommandBuffer(vkCommandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    // TODO: Delete these, and put it in a shader class.
    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    inline VkShaderModule createShaderModule(VkDevice vkDevice, const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const uint32_t*>(code.data())
        };

        VkShaderModule shaderModule;
        vkCreateShaderModule(vkDevice, &createInfo, nullptr, &shaderModule);
        return shaderModule;
    }

    inline VkRenderPass createRenderPass(VkDevice vkDevice, VkFormat vkFormat) {
        VkAttachmentDescription colorAttachment {
            .flags{},
            .format = vkFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference colorAttachmentRef {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription vkSubpassDescription {
            .flags{},
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount{},
            .pInputAttachments{},
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pResolveAttachments{},
            .pDepthStencilAttachment{},
            .preserveAttachmentCount{},
            .pPreserveAttachments{}
        };

        VkSubpassDependency vkSubpassDependency {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // waiting for operation start
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // operations that should wait for end
            .srcAccessMask = 0, // access
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // we need to be able to write to this stage
            .dependencyFlags{}
        };

        VkRenderPassCreateInfo renderPassInfo {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .pNext{},
                .flags{},
                .attachmentCount = 1,
                .pAttachments = &colorAttachment,
                .subpassCount = 1,
                .pSubpasses = &vkSubpassDescription,
                .dependencyCount = 1,
                .pDependencies = &vkSubpassDependency,
        };

        VkRenderPass vkRenderPass;
        vkCreateRenderPass(vkDevice, &renderPassInfo, nullptr, &vkRenderPass);
        return vkRenderPass;
    }

    inline void deleteRenderPass(VkDevice vkDevice, VkRenderPass vkRenderPass) {
        vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
    }

    inline VkPipeline createGraphicsPipeline(VkDevice vkDevice, VkExtent2D extent, VkRenderPass vkRenderPass, VkPipelineLayout& vkPipelineLayout, VkPipeline& vkPipeline) {
        auto vertShaderCode = readFile("dat/shaders/default.vert.glsl.spv");
        auto fragShaderCode = readFile("dat/shaders/default.frag.glsl.spv");

        VkShaderModule vertShaderModule = createShaderModule(vkDevice, vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(vkDevice, fragShaderCode);

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

        // end of shaders.

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        void* bindingDescription = nullptr;//Vertex::getBindingDescription();
        void* attributeDescriptions = nullptr; //Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;

        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        VkViewport viewport {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float) extent.width,
            .height = (float) extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        VkRect2D scissor {
            .offset = {0, 0},
            .extent = extent
        };

        VkPipelineViewportStateCreateInfo viewportState {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor
        };

        VkPipelineRasterizationStateCreateInfo rasterizer {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext{},
            .flags{},
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor{},
            .depthBiasClamp{},
            .depthBiasSlopeFactor{},
            .lineWidth = 1.0f
        };

        VkPipelineMultisampleStateCreateInfo multisampling {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext{},
            .flags{},
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading{},
            .pSampleMask{},
            .alphaToCoverageEnable{},
            .alphaToOneEnable{}
        };

        VkPipelineDepthStencilStateCreateInfo depthStencil {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
        };

        VkPipelineColorBlendAttachmentState colorBlendAttachment {
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor{},
            .dstColorBlendFactor{},
            .colorBlendOp{},
            .srcAlphaBlendFactor{},
            .dstAlphaBlendFactor{},
            .alphaBlendOp{},
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo colorBlending {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags{},
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &colorBlendAttachment,
                .blendConstants { 0.0f, 0.0f, 0.0f, 0.0f }
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext{},
            .flags{},
            .setLayoutCount{}, // 1
            .pSetLayouts{}, // &vkDescriptorSetLayout,
            .pushConstantRangeCount{},
            .pPushConstantRanges{},
        };

        vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &vkPipelineLayout);

        VkGraphicsPipelineCreateInfo pipelineCreateInfo {
           .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
           .pNext{},
           .flags{},
           .stageCount = 2,
           .pStages = shaderStages,
           .pVertexInputState = &vertexInputInfo,
           .pInputAssemblyState = &inputAssembly,
           .pTessellationState{},
           .pViewportState = &viewportState,
           .pRasterizationState = &rasterizer,
           .pMultisampleState = &multisampling,
           .pDepthStencilState = &depthStencil,
           .pColorBlendState = &colorBlending,
           .pDynamicState{},
           .layout = vkPipelineLayout,
           .renderPass = vkRenderPass,
           .subpass = 0,
           .basePipelineHandle = VK_NULL_HANDLE,
           .basePipelineIndex{}
        };

        if (vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &vkPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline!");
        }

        vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);

        return vkPipeline;
    }

    inline void deletePipelineLayout(VkDevice vkDevice, VkPipelineLayout vkPipelineLayout) {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
    }

    inline void deletePipeline(VkDevice vkDevice, VkPipeline vkPipeline) {
        vkDestroyPipeline(vkDevice, vkPipeline, nullptr);
    }

    void createDescriptorSetLayout(VkDevice vkDevice) {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, nullptr, &vkDescriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    inline VkFramebuffer createFramebuffer(VkDevice vkDevice, VkRenderPass vkRenderPass, uint32_t width, uint32_t height, vector<VkImageView> vkImageViews) {
        VkFramebufferCreateInfo framebufferInfo {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext{},
                .flags{},
                .renderPass = vkRenderPass,
                .attachmentCount = static_cast<uint32_t>(vkImageViews.size()),
                .pAttachments = vkImageViews.data(),
                .width = width,
                .height = height,
                .layers = 1
        };

        VkFramebuffer vkFramebuffer;
        vkCreateFramebuffer(vkDevice, &framebufferInfo, nullptr, &vkFramebuffer);
        return vkFramebuffer;
    }

    inline void deleteFramebuffer(VkDevice vkDevice, VkFramebuffer vkFramebuffer) {
        vkDestroyFramebuffer(vkDevice, vkFramebuffer, nullptr);
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

    inline void printQueueFamilies(VkPhysicalDevice vkPhysicalDevice) {
        int i = 0;
        for (const auto& a : getQueueFamilyProperties(vkPhysicalDevice)) {
            info("family {}, queue count: {}", i, a.queueCount);

            if (VK_QUEUE_GRAPHICS_BIT & a.queueFlags) info("graphics !");
            if (VK_QUEUE_COMPUTE_BIT & a.queueFlags) info("compute !");
            if (VK_QUEUE_TRANSFER_BIT & a.queueFlags) info("transfer !");
            if (VK_QUEUE_SPARSE_BINDING_BIT & a.queueFlags) info("sparse !");

            i++;
        }
    }
};
#pragma once

#include "glfw_vulkan.h"

namespace VK {
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;


    struct Queues {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;

        [[nodiscard]] bool isComplete() const {
            return graphics.has_value() && present.has_value();
        }

        [[nodiscard]] set<uint32_t> unique_set() const {
            return set<uint32_t> {graphics.value(), present.value()};
        }

        static force_inline vector<VkQueueFamilyProperties>
        getQueueFamilyProperties(VkPhysicalDevice vkPhysicalDevice) {
            uint32_t count;
            vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, nullptr);
            vector<VkQueueFamilyProperties> queueFamilyProperties(count);
            vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, queueFamilyProperties.data());
            return queueFamilyProperties;
        }

        // select the first queue that has present & graphics abilities
        force_inline void getQueueFamilyIndices(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface,
                                                vector<VkQueueFamilyProperties> vkQueueFamilyProperties =
                                                vector<VkQueueFamilyProperties> {0}) {
            if (vkQueueFamilyProperties.empty()) vkQueueFamilyProperties = getQueueFamilyProperties(vkPhysicalDevice);

            int i = 0;
            for (const auto& queueFamily: vkQueueFamilyProperties) {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentSupport);

                if (presentSupport) present = i;
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) graphics = i;
                if (isComplete()) break;
                i++;
            }
        }

        force_inline void getQueueFamilyIndices(vector<VkQueueFamilyProperties> vkQueueFamilyProperties =
        vector<VkQueueFamilyProperties> {0}) {
            getQueueFamilyIndices(physicalDevice, surface.surface, vkQueueFamilyProperties);
        }

        force_inline vector<VkDeviceQueueCreateInfo> getQueueCreateInfos() {
            vector<VkDeviceQueueCreateInfo> vkDeviceQueueCreateInfos {};

            float queuePriority = 0.5f;
            for (uint32_t queueFamilyIndex: unique_set()) {
                vkDeviceQueueCreateInfos.push_back(VkDeviceQueueCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext{},
                    .flags{},
                    .queueFamilyIndex = queueFamilyIndex,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority
                });
            }

            return vkDeviceQueueCreateInfos;
        }
    };

    Queues queues;

    struct Surface {
        VkSurfaceKHR surface = nullptr;
        VkSurfaceCapabilitiesKHR vkSurfaceCapabilities {};
        vector<VkSurfaceFormatKHR> vkSurfaceFormats;
        vector<VkPresentModeKHR> vkPresentModes;

        VkSurfaceFormatKHR vkSurfaceFormat;
        VkPresentModeKHR vkPresentMode;

        static force_inline vector<VkSurfaceFormatKHR>
        enumerateSurfaceFormats(VkPhysicalDevice vkDevice, VkSurfaceKHR vkSurface) {
            uint32_t count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice, vkSurface, &count, nullptr);
            vector<VkSurfaceFormatKHR> surfaceFormats(count);
            if (count != 0) vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice, vkSurface, &count, surfaceFormats.data());
            return surfaceFormats;
        }

        static force_inline vector<VkPresentModeKHR>
        enumeratePresentModes(VkPhysicalDevice vkDevice, VkSurfaceKHR vkSurface) {
            uint32_t count;
            vkGetPhysicalDeviceSurfacePresentModesKHR(vkDevice, vkSurface, &count, nullptr);
            vector<VkPresentModeKHR> presentModes(count);
            if (count != 0) vkGetPhysicalDeviceSurfacePresentModesKHR(vkDevice, vkSurface, &count, presentModes.data());

            return presentModes;
        }

        static force_inline VkSurfaceCapabilitiesKHR
        getSurfaceCapabilities(VkPhysicalDevice vkDevice, VkSurfaceKHR vkSurface) {
            VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkDevice, vkSurface, &vkSurfaceCapabilitiesKHR);
            return vkSurfaceCapabilitiesKHR;
        }

        static force_inline Surface getSurface(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface) {
            Surface details;

            details.vkSurfaceCapabilities = getSurfaceCapabilities(vkPhysicalDevice, vkSurface);
            details.vkSurfaceFormats = enumerateSurfaceFormats(vkPhysicalDevice, vkSurface);
            details.vkPresentModes = enumeratePresentModes(vkPhysicalDevice, vkSurface);

            return details;
        }

        [[nodiscard]] force_inline Surface getSurface() const {
            return getSurface(physicalDevice, surface);
        }

        static force_inline VkSurfaceFormatKHR
        chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats,
                                VkFormat format = VK_FORMAT_B8G8R8A8_SRGB,
                                VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            for (const auto& availableFormat: availableFormats) {
                if (availableFormat.format == format && availableFormat.colorSpace == colorSpace) {
                    return availableFormat;
                }
            }

            return availableFormats[0];
        }

        static force_inline VkPresentModeKHR
        chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes,
                              VkPresentModeKHR mode = VK_PRESENT_MODE_MAILBOX_KHR) {
            for (const auto& availablePresentMode: availablePresentModes) {
                if (availablePresentMode == mode) {
                    return availablePresentMode;
                }
            }

            return VK_PRESENT_MODE_FIFO_KHR;
        }

        static force_inline VkExtent2D
        chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            } else {
                VkExtent2D actualExtent = {width, height};

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                                capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                                 capabilities.maxImageExtent.height);

                return actualExtent;
            }
        }

        force_inline VkSwapchainKHR
        createSwapchain(VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, VkSurfaceKHR vkSurface,
                        uint32_t width, uint32_t height, VkSurfaceFormatKHR surfaceFormat,
                        VkPresentModeKHR presentMode, VkExtent2D extent) {
            uint32_t imageCount = vkSurfaceCapabilities.minImageCount + 1;
            if (vkSurfaceCapabilities.maxImageCount > 0 // 0 is a special case for no limits
                && imageCount > vkSurfaceCapabilities.maxImageCount) {
                imageCount = vkSurfaceCapabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext{},
                .flags{},
                .surface = vkSurface,
                .minImageCount = imageCount,
                .imageFormat = surfaceFormat.format,
                .imageColorSpace = surfaceFormat.colorSpace,
                .imageExtent = extent,
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .imageSharingMode{},
                .queueFamilyIndexCount{},
                .pQueueFamilyIndices{},
                .preTransform = vkSurfaceCapabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = presentMode,
                .clipped = VK_TRUE,
                .oldSwapchain{}
            };

            uint32_t idx[] = {queues.graphics.value(), queues.present.value()};

            if (queues.graphics != queues.present) {
                vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                vkSwapchainCreateInfoKHR.queueFamilyIndexCount = 2;
                vkSwapchainCreateInfoKHR.pQueueFamilyIndices = idx;
            } else {
                vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // best perf !
            }

            VkSwapchainKHR vkSwapchainKHR;
            vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, nullptr, &vkSwapchainKHR);
            return vkSwapchainKHR;
        }

        force_inline void init() {
            getSurface();
        }
    };

    Surface surface;
}

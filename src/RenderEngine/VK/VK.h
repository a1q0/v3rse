#pragma once

#include "glfw_vulkan.h"
#include "using_std.h"
#include "logging.h"

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
#include <optional>

#include "VK_GLOBALS.h"
#include "VK_DFL.h"
#include "VK_DBG.h"
#include "VK_MEM.h"
#include "VK_INI.h"

namespace VK {

    struct Image {
        VkImage image;
        VkFormat format;
        VkExtent2D extent;

        VkImageView view;

        static force_inline VkImageView
        createImageView(VkDevice vkDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
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

        static force_inline void deleteImageView(VkDevice vkDevice, VkImageView vkImageView) {
            vkDestroyImageView(vkDevice, vkImageView, nullptr);
        }

        static force_inline void deleteImage(VkDevice vkDevice, VkImage vkImage) {
            vkDestroyImage(vkDevice, vkImage, nullptr);
        }
    };

    force_inline VkCommandPool createCommandPool(VkDevice vkDevice, Queues queueFamilyIndices) {
        VkCommandPoolCreateInfo vkCommandPoolCreateInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext{},
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queueFamilyIndices.graphics.id.value()
        };

        VkCommandPool vkCommandPool;
        vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, nullptr, &vkCommandPool);
        return vkCommandPool;
    }

    force_inline void deleteCommandPool(VkDevice vkDevice, VkCommandPool vkCommandPool) {
        vkDestroyCommandPool(vkDevice, vkCommandPool, nullptr);
    }

    force_inline void recordCommandBuffer(VkCommandBuffer vkCommandBuffer, VkRenderPass vkRenderPass,
                                          VkFramebuffer vkFramebuffer,
                                          VkExtent2D extent, VkPipeline vkPipeline) {
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

    force_inline VkShaderModule createShaderModule(VkDevice vkDevice, const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const uint32_t*>(code.data())
        };

        VkShaderModule shaderModule;
        vkCreateShaderModule(vkDevice, &createInfo, nullptr, &shaderModule);
        return shaderModule;
    }

    force_inline VkDescriptorSetLayout createDescriptorSetLayout(VkDevice vkDevice) {
        VkDescriptorSetLayoutBinding uboLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        };

        VkDescriptorSetLayoutBinding samplerLayoutBinding {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        };

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext{},
            .flags{},
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data(),
        };

        VkDescriptorSetLayout vkDescriptorSetLayout;
        if (vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, nullptr, &vkDescriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        return vkDescriptorSetLayout;
    }

    force_inline VkFramebuffer
    createFramebuffer(VkDevice vkDevice, VkRenderPass vkRenderPass, uint32_t width, uint32_t height,
                      vector<VkImageView> vkImageViews) {
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

    force_inline void deleteFramebuffer(VkDevice vkDevice, VkFramebuffer vkFramebuffer) {
        vkDestroyFramebuffer(vkDevice, vkFramebuffer, nullptr);
    }

    struct Queue {
        std::optional<uint32_t> id;
        VkQueue vkQueue;
    };

    struct Queues {
        Queue graphics;
        Queue present;

        [[nodiscard]] bool isComplete() const {
            return graphics.id.has_value() && present.id.has_value();
        }

        [[nodiscard]] set<uint32_t> unique_set() const {
            return set<uint32_t> {graphics.id.value(), present.id.value()};
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

                if (presentSupport) present.id = i;
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) graphics.id = i;
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

        force_inline void printQueueFamilies(VkPhysicalDevice vkPhysicalDevice) {
            int i = 0;
            for (const auto& a: Queues::getQueueFamilyProperties(vkPhysicalDevice)) {
                info("family {}, queue count: {}", i, a.queueCount);

                if (VK_QUEUE_GRAPHICS_BIT & a.queueFlags) info("graphics !");
                if (VK_QUEUE_COMPUTE_BIT & a.queueFlags) info("compute !");
                if (VK_QUEUE_TRANSFER_BIT & a.queueFlags) info("transfer !");
                if (VK_QUEUE_SPARSE_BINDING_BIT & a.queueFlags) info("sparse !");

                i++;
            }
        }
    } queues;

    class Surface {
    public:
        VkSurfaceKHR surface = nullptr;
        VkSurfaceCapabilitiesKHR vkSurfaceCapabilities {};
        vector<VkSurfaceFormatKHR> vkSurfaceFormats;
        vector<VkPresentModeKHR> vkPresentModes;

        VkSurfaceFormatKHR vkSurfaceFormat {};
        VkPresentModeKHR vkPresentMode {};

        VkExtent2D extent {};

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

        [[nodiscard]] force_inline vector<VkPresentModeKHR> enumeratePresentModes() const {
            enumeratePresentModes(physicalDevice, surface);
        }

        static force_inline VkSurfaceCapabilitiesKHR
        getSurfaceCapabilities(VkPhysicalDevice vkDevice, VkSurfaceKHR vkSurface) {
            VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkDevice, vkSurface, &vkSurfaceCapabilitiesKHR);
            return vkSurfaceCapabilitiesKHR;
        }

        [[nodiscard]] force_inline VkSurfaceCapabilitiesKHR getSurfaceCapabilities() const {
            getSurfaceCapabilities(physicalDevice, surface);
        }

        force_inline VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats,
                                                                VkFormat format = VK_FORMAT_B8G8R8A8_SRGB,
                                                                VkColorSpaceKHR colorSpace
                                                                = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            for (const auto& availableFormat: availableFormats) {
                if (availableFormat.format == format && availableFormat.colorSpace == colorSpace) {
                    return availableFormat;
                }
            }

            return vkSurfaceFormat = availableFormats[0];
        }

        force_inline VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR mode = VK_PRESENT_MODE_MAILBOX_KHR) {

            for (const auto& availablePresentMode: enumeratePresentModes()) {
                if (availablePresentMode == mode) {
                    return vkPresentMode = availablePresentMode;
                }
            }

            return vkPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        }

        force_inline VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                                                 uint32_t width, uint32_t height) {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            } else {
                VkExtent2D actualExtent = {width, height};

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                                capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                                 capabilities.maxImageExtent.height);

                return extent = actualExtent;
            }
        }

        force_inline void init() {
            vkSurfaceCapabilities = getSurfaceCapabilities(physicalDevice, surface);
            vkSurfaceFormats = enumerateSurfaceFormats(physicalDevice, surface);
            vkPresentModes = enumeratePresentModes(physicalDevice, surface);
            swapchain.surface = this;

        }

        force_inline void destroy() const {
            vkDestroySurfaceKHR(VK::instance, surface, nullptr);
        }

        struct Swapchain {
            Surface* surface = nullptr;
            VkSwapchainKHR vkSwapchainKHR = nullptr;

            vector<VkFramebuffer> swapchain_framebuffers;
            vector<Image> images;

            VkSwapchainKHR createSwapchain(VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice,
                                           VkSurfaceKHR vkSurface, uint32_t width, uint32_t height,
                                           VkSurfaceFormatKHR surfaceFormat,
                                           VkPresentModeKHR presentMode) {
                uint32_t imageCount = surface->vkSurfaceCapabilities.minImageCount + 1;
                if (surface->vkSurfaceCapabilities.maxImageCount > 0 // 0 is a special case for no limits
                    && imageCount > surface->vkSurfaceCapabilities.maxImageCount) {
                    imageCount = surface->vkSurfaceCapabilities.maxImageCount;
                }

                VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR {
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                    .pNext{},
                    .flags{},
                    .surface = vkSurface,
                    .minImageCount = imageCount,
                    .imageFormat = surfaceFormat.format,
                    .imageColorSpace = surfaceFormat.colorSpace,
                    .imageExtent = surface->extent,
                    .imageArrayLayers = 1,
                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    .imageSharingMode{},
                    .queueFamilyIndexCount{},
                    .pQueueFamilyIndices{},
                    .preTransform = surface->vkSurfaceCapabilities.currentTransform,
                    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                    .presentMode = presentMode,
                    .clipped = VK_TRUE,
                    .oldSwapchain{}
                };

                uint32_t idx[] = {queues.graphics.id.value(), queues.present.id.value()};

                if (queues.graphics.id.value() != queues.present.id.value()) {
                    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                    vkSwapchainCreateInfoKHR.queueFamilyIndexCount = 2;
                    vkSwapchainCreateInfoKHR.pQueueFamilyIndices = idx;
                } else {
                    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // best perf !
                }

                vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, nullptr, &vkSwapchainKHR);
                return vkSwapchainKHR;
            }

            VkSwapchainKHR createSwapchain(uint32_t width, uint32_t height) {
                surface->chooseSwapExtent(surface->vkSurfaceCapabilities, width, height);
                return createSwapchain(physicalDevice, device, surface->surface, width, height,
                                       surface->vkSurfaceFormat,
                                       surface->vkPresentMode);
            }

            force_inline void create(VkDevice vkDevice) {
                uint32_t count;
                vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &count, nullptr);
                vector<VkImage> vkImages(count);
                vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &count, vkImages.data());

                images.resize(count);
                for (int i = 0; i < images.size(); i++) {
                    images[i].image = vkImages[i];
                    images[i].view = VK::Image.createImageView(VK::device, swapchain_images[i], swapchain_images_format,
                                                               VK_IMAGE_ASPECT_COLOR_BIT);
                }

                vkGetDeviceQueue(VK::device, VK::queues.graphics.id.value(), 0, &VK::queues.graphics.vkQueue);
                vkGetDeviceQueue(VK::device, VK::queues.present.id.value(), 0, &VK::queues.present.vkQueue);

                if (VK::queues.graphics.vkQueue == VK::queues.present.vkQueue)
                    info("using same queue for graphics and presentation");
            }

            force_inline void create() {
                create(device);
            }

            void destroy() {
                vkDestroySwapchainKHR(device, vkSwapchainKHR, nullptr);

                for (const auto& iv: images) {
                    //VK::image.deleteImageView(VK::device, iv);
                }
            }
        } swapchain;
    } surface;

    struct RenderPass {

        VkRenderPass renderPass;

        force_inline VkRenderPass createRenderPass(VkDevice vkDevice, VkFormat vkFormat) {
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

            vkCreateRenderPass(vkDevice, &renderPassInfo, nullptr, &renderPass);
        }

        force_inline VkRenderPass createRenderPass() {
            createRenderPass(device, surface.vkSurfaceFormat.format);
        }

        force_inline void deleteRenderPass(VkDevice vkDevice, VkRenderPass vkRenderPass) {
            vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
        }

    } renderPass;

    struct Pipeline {
        VkPipelineLayout layout;
        VkPipeline pipeline;

        force_inline VkPipeline createGraphicsPipeline(VkDevice vkDevice, VkExtent2D extent, VkRenderPass vkRenderPass,
                                                       VkPipelineLayout& vkPipelineLayout, VkPipeline& vkPipeline) {
            auto vertShaderCode = readFile("dat/shaders/default.vert.glsl.spv");
            auto fragShaderCode = readFile("dat/shaders/default.frag.glsl.spv");

            VkShaderModule vertShaderModule = createShaderModule(vkDevice, vertShaderCode);
            VkShaderModule fragShaderModule = createShaderModule(vkDevice, fragShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

            // end of shaders.

            VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            void* bindingDescription = nullptr;//Vertex::getBindingDescription();
            void* attributeDescriptions = nullptr; //Vertex::getAttributeDescriptions();

            vertexInputInfo.vertexBindingDescriptionCount = 0;
            vertexInputInfo.pVertexBindingDescriptions = nullptr;

            vertexInputInfo.vertexAttributeDescriptionCount = 0;
            vertexInputInfo.pVertexAttributeDescriptions = nullptr;

            VkPipelineInputAssemblyStateCreateInfo inputAssembly {
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
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                  VK_COLOR_COMPONENT_A_BIT
            };

            VkPipelineColorBlendStateCreateInfo colorBlending {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags{},
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &colorBlendAttachment,
                .blendConstants {0.0f, 0.0f, 0.0f, 0.0f}
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

            CHECK(vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &vkPipeline),
                  "failed to create graphics pipeline.");

            vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);
            vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);

            return vkPipeline;
        }

        force_inline VkPipeline createGraphicsPipeline() {
            createGraphicsPipeline(device, surface.extent, renderPass.renderPass, layout, pipeline);
        }

        force_inline void deletePipelineLayout(VkDevice vkDevice, VkPipelineLayout vkPipelineLayout) {
            vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
        }

        force_inline void deletePipeline(VkDevice vkDevice, VkPipeline vkPipeline) {
            vkDestroyPipeline(vkDevice, vkPipeline, nullptr);
        }

    } pipeline;
};
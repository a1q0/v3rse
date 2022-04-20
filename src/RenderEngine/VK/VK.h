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

namespace VK {

//    class VKO {
//    public:
//        void* handle = nullptr;
//
//        VKO(void* handle) {
//            this->handle = handle;
//            info("hello");
//        }
//
//        VKO(void) {
//            this->create();
//        }
//
//        force_inline virtual void create() = 0;
//
//        force_inline virtual void destroy() = 0;
//    };

    class Framebuffer {
    public:
        VkFramebuffer framebuffer;

        force_inline VkFramebuffer
        create(VkRenderPass vkRenderPass, uint32_t width, uint32_t height,
               vector<VkImageView> attachments) {
            VkFramebufferCreateInfo framebufferInfo {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext{},
                .flags{},
                .renderPass = vkRenderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = width,
                .height = height,
                .layers = 1
            };

            vkCreateFramebuffer(VK::device, &framebufferInfo, nullptr, &framebuffer);
            return framebuffer;
        }

        force_inline void destroy() {
            vkDestroyFramebuffer(VK::device, framebuffer, nullptr);
        }
    };


    class Image {
    public:
        VkImage image {};
        VkFormat format {};
        VkExtent2D extent {};
        VkImageView view {};

        static force_inline VkImageView
        createView(VkDevice vkDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
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


        force_inline void destroy() const {
            vkDestroyImageView(device, view, nullptr);
            vkDestroyImage(device, image, nullptr);
        }
    };

    class Frame : public Image {
    public:
        Framebuffer framebuffer {};
    };


    struct Queue {
        std::optional<uint32_t> id;
        VkQueue vkQueue;
    };

    struct Queues {
        Queue graphics;
        Queue present;

        void init() {
            vkGetDeviceQueue(VK::device, graphics.id.value(), 0, &graphics.vkQueue);
            vkGetDeviceQueue(VK::device, present.id.value(), 0, &present.vkQueue);

            if (graphics.vkQueue == present.vkQueue)
                info("using same queue for graphics and presentation");
        }

        [[nodiscard]] bool isComplete() const {
            return graphics.id.has_value() && present.id.has_value();
        }

        [[nodiscard]] set<uint32_t> unique_set() const {
            return set<uint32_t> {graphics.id.value(), present.id.value()};
        }

        // select the first queue that has present & graphics abilities
        force_inline void getQueueFamilyIndices(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface,
                                                vector<VkQueueFamilyProperties> vkQueueFamilyProperties =
                                                vector<VkQueueFamilyProperties> {0}) {
            uint32_t count;
            vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, nullptr);
            vector<VkQueueFamilyProperties> queueFamilyProperties(count);
            vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, queueFamilyProperties.data());

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

        force_inline void
        getQueueFamilyIndices(VkSurfaceKHR vkSurfaceKHR, vector<VkQueueFamilyProperties> vkQueueFamilyProperties =
        vector<VkQueueFamilyProperties> {0}) {
            getQueueFamilyIndices(physicalDevice, vkSurfaceKHR, vkQueueFamilyProperties);
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
    } queues;

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

    class Surface {
    public:
        VkSurfaceKHR surface = nullptr;
        VkSurfaceCapabilitiesKHR vkSurfaceCapabilities {};
        vector<VkSurfaceFormatKHR> surfaceFormats;
        vector<VkPresentModeKHR> presentModes;

        VkSurfaceFormatKHR vkSurfaceFormat {};
        VkPresentModeKHR vkPresentMode {};

        VkExtent2D extent {};

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

        force_inline VkSurfaceKHR create() {
            return createSurface(VK::instance, VK::window);
        }

        force_inline void destroy() const {
            vkDestroySurfaceKHR(VK::instance, surface, nullptr);
        }

        force_inline void enumerateSurfaceFormats() {
            uint32_t count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(VK::physicalDevice, surface, &count, nullptr);
            surfaceFormats.resize(count);
            if (count != 0)
                vkGetPhysicalDeviceSurfaceFormatsKHR(VK::physicalDevice, surface, &count, surfaceFormats.data());
        }

        force_inline void enumeratePresentModes() {
            uint32_t count;
            vkGetPhysicalDeviceSurfacePresentModesKHR(VK::physicalDevice, surface, &count, nullptr);
            presentModes.resize(count);
            if (count != 0)
                vkGetPhysicalDeviceSurfacePresentModesKHR(VK::physicalDevice, surface, &count, presentModes.data());
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

        force_inline VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkFormat format = VK_FORMAT_B8G8R8A8_SRGB,
                                                                VkColorSpaceKHR colorSpace
                                                                = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return chooseSwapSurfaceFormat(surfaceFormats, format, colorSpace);
        }

        force_inline VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR mode = VK_PRESENT_MODE_MAILBOX_KHR) {

            for (const auto& availablePresentMode: presentModes) {
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
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VK::physicalDevice, surface, &vkSurfaceCapabilities);
            enumerateSurfaceFormats();
            enumeratePresentModes();
            chooseSwapSurfaceFormat();
            chooseSwapPresentMode();
            swapchain.surface = this;

        }

        struct Swapchain {
            Surface* surface = nullptr;
            VkSwapchainKHR swapchain = nullptr;

            vector<Frame> frames;

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

                vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, nullptr, &swapchain);
                return swapchain;
            }

            VkSwapchainKHR createSwapchain(uint32_t width, uint32_t height) {
                surface->chooseSwapExtent(surface->vkSurfaceCapabilities, width, height);
                return createSwapchain(physicalDevice, device, surface->surface, width, height,
                                       surface->vkSurfaceFormat,
                                       surface->vkPresentMode);
            }

            force_inline void create() {
                uint32_t count;
                vkGetSwapchainImagesKHR(VK::device, swapchain, &count, nullptr);
                vector<VkImage> vkImages(count);
                vkGetSwapchainImagesKHR(VK::device, swapchain, &count, vkImages.data());

                frames.resize(count);
                for (int i = 0; i < frames.size(); i++) {
                    frames[i].image = vkImages[i];
                    frames[i].view = VK::Image::createView(VK::device, frames[i].image, frames[i].format,
                                                           VK_IMAGE_ASPECT_COLOR_BIT);
                }
            }

            void destroy() {
                vkDestroySwapchainKHR(device, swapchain, nullptr);

                for (const auto& iv: frames) {
                    //VK::image.deleteImageView(VK::device, iv);
                }
            }
        } swapchain;

        force_inline void destroy() {
            swapchain.destroy();
            vkDestroySurfaceKHR(VK::instance, surface, nullptr);
        }
    } surface;

    struct RenderPass {
        VkRenderPass renderPass;

        force_inline void createRenderPass(VkFormat vkFormat) {
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

            vkCreateRenderPass(VK::device, &renderPassInfo, nullptr, &renderPass);
        }

        force_inline void deleteRenderPass(VkDevice vkDevice, VkRenderPass vkRenderPass) {
            vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
        }

    } renderPass;

    struct Pipeline {
        VkPipelineLayout layout;
        VkPipeline pipeline;

        force_inline void createGraphicsPipeline(VkDevice vkDevice, VkExtent2D extent, VkRenderPass vkRenderPass) {
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

            vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &layout);

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
                .layout = layout,
                .renderPass = vkRenderPass,
                .subpass = 0,
                .basePipelineHandle = VK_NULL_HANDLE,
                .basePipelineIndex{}
            };

            CHECK(vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline),
                  "failed to create graphics pipeline.");

            vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);
            vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);
        }

        force_inline VkPipeline createGraphicsPipeline() {
            createGraphicsPipeline(device, surface.extent, renderPass.renderPass);
        }

        force_inline void deletePipelineLayout(VkDevice vkDevice, VkPipelineLayout vkPipelineLayout) {
            vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
        }

        force_inline void deletePipeline(VkDevice vkDevice, VkPipeline vkPipeline) {
            vkDestroyPipeline(vkDevice, vkPipeline, nullptr);
        }

    } pipeline;

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
            swapChainAdequate = !surface.surfaceFormats.empty() && !surface.presentModes.empty();
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
        return getBestPhysicalDevice(VK::instance, VK::surface.surface);
    }

    force_inline void init() {
        VK::createInstance();
        VK::createDebugMessenger();
        VK::surface.create();
        VK::getBestPhysicalDevice();
        VK::queues.getQueueFamilyIndices(surface.surface);
        VK::device = VK::createLogicalDevice();
        VK::surface.init();
    }
};
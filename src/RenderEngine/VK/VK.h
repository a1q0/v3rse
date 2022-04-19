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

    force_inline VkImageView
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

    force_inline void deleteImageView(VkDevice vkDevice, VkImageView vkImageView) {
        vkDestroyImageView(vkDevice, vkImageView, nullptr);
    }

    force_inline void deleteImage(VkDevice vkDevice, VkImage vkImage) {
        vkDestroyImage(vkDevice, vkImage, nullptr);
    }

    force_inline VkCommandPool createCommandPool(VkDevice vkDevice, Queues queueFamilyIndices) {
        VkCommandPoolCreateInfo vkCommandPoolCreateInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext{},
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queueFamilyIndices.graphics.value()
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

        VkRenderPass vkRenderPass;
        vkCreateRenderPass(vkDevice, &renderPassInfo, nullptr, &vkRenderPass);
        return vkRenderPass;
    }

    force_inline void deleteRenderPass(VkDevice vkDevice, VkRenderPass vkRenderPass) {
        vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
    }

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

    force_inline void deletePipelineLayout(VkDevice vkDevice, VkPipelineLayout vkPipelineLayout) {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
    }

    force_inline void deletePipeline(VkDevice vkDevice, VkPipeline vkPipeline) {
        vkDestroyPipeline(vkDevice, vkPipeline, nullptr);
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

    force_inline void deleteSwapchain(VkDevice vkDevice, VkSwapchainKHR vkSwapchain) {
        vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);
    }

    force_inline vector<VkImage> getSwapchainImages(VkDevice vkDevice, VkSwapchainKHR vkSwapchainKHR) {
        uint32_t count;
        vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &count, nullptr);
        vector<VkImage> vkImages(count);
        vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &count, vkImages.data());
        return vkImages;
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
};
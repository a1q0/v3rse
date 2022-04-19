#include "RenderEngine.h"

#include "glfw_vulkan.h"
#include "using_std.h"
#include "logging.h"
#include "VK/VK.h"

VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkPipeline pipeline;

VkCommandBuffer commandBuffer = nullptr;
VkCommandPool commandPool = nullptr;
VkQueue queue_graphics = nullptr;
VkQueue queue_present = nullptr;

VkFence inFlightFence;
VkSemaphore imageAvailableSemaphores;
VkSemaphore renderFinishedSemaphores;

//vector<Vertex> vertices = {{{0.0f,  -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
//                           {{0.5f,  0.5f,  0.0f}, {0.0f, 1.0f, 0.0f}},
//                           {{-0.5f, 0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}}};

#include "benchmark.h"

void RenderEngine::init() {
    window_create(width, height, "v3rse");

    VK::init();

    int fb_width;
    int fb_height;

    window_framebuffer_size(fb_width, fb_height);

    VK::surface.swapchain.createSwapchain(width, height);

    VK::renderPass.createRenderPass();
    VK::pipeline.createGraphicsPipeline();

    swapchain_framebuffers.resize(swapchain_images.size());
    for (int i = 0; i < swapchain_images.size(); i++) {
        swapchain_framebuffers[i] = VK::createFramebuffer(VK::device, renderPass, extent.width, extent.height,
                                                          {swapchain_images_view[i]});
    }

    commandPool = VK::createCommandPool(VK::device, queueFamilyIndices);

    VkCommandBufferAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext{},
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    vkAllocateCommandBuffers(VK::device, &allocInfo, &commandBuffer);

    VkSemaphoreCreateInfo semaphoreInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    if (vkCreateSemaphore(VK::device, &semaphoreInfo, nullptr, &imageAvailableSemaphores) != VK_SUCCESS ||
        vkCreateSemaphore(VK::device, &semaphoreInfo, nullptr, &renderFinishedSemaphores) != VK_SUCCESS ||
        vkCreateFence(VK::device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }

    VkPhysicalDeviceMemoryProperties memoryProperties = VK::getPhysicalDeviceMemoryProperties(VK::physicalDevice);
    //Vertex::createVertexBuffer(VK::device, vertices, VK_SHARING_MODE_EXCLUSIVE);
}

void RenderEngine::loop() {
    auto last = Clock::now();
    auto delta = last - Clock::now();

    while (!window_is_closed()) {
        delta = Clock::now() - last;
        last = Clock::now();

        auto seconds = 1e9 / std::chrono::duration_cast<std::chrono::nanoseconds>(delta).count();

        //info("{:02f} fps", (seconds));

        frame();
        window_update();
    }
    vkDeviceWaitIdle(VK::device);
}

void RenderEngine::frame() {
    vkWaitForFences(VK::device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result_acquireNextImage = vkAcquireNextImageKHR(VK::device, swapchain, UINT64_MAX,
                                                             imageAvailableSemaphores,
                                                             VK_NULL_HANDLE, &imageIndex);
    if (result_acquireNextImage == VK_ERROR_OUT_OF_DATE_KHR) {
        info("window resized ? swapchain out of date.");
        return;
    }

    vkResetFences(VK::device, 1, &inFlightFence);

    vkResetCommandBuffer(commandBuffer, 0); /*VkCommandBufferResetFlagBits*/
    VK::recordCommandBuffer(commandBuffer, renderPass, swapchain_framebuffers[imageIndex],
                            swapchain_images_extent, pipeline);

    VkPipelineStageFlags stageFlags[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext{},
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &imageAvailableSemaphores,
        .pWaitDstStageMask = stageFlags,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderFinishedSemaphores
    };

    vkQueueSubmit(queue_graphics, 1, &submitInfo, inFlightFence);

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext{},
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinishedSemaphores,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
        .pResults{}
    };

    vkQueuePresentKHR(queue_present, &presentInfo);
}

void RenderEngine::exit() {

    vkDestroyFence(VK::device, inFlightFence, nullptr);
    vkDestroySemaphore(VK::device, renderFinishedSemaphores, nullptr);
    vkDestroySemaphore(VK::device, imageAvailableSemaphores, nullptr);

    for (const auto& f: swapchain_framebuffers) {
        VK::deleteFramebuffer(VK::device, f);
    }

    VK::swapchain.destroy();

    VK::deleteCommandPool(VK::device, commandPool);
    VK::renderPass.deleteRenderPass(VK::device, renderPass);
    VK::deletePipelineLayout(VK::device, pipelineLayout);
    VK::pipeline.deletePipeline(VK::device, pipeline);
    VK::deleteSwapchain(VK::device, swapchain);
    VK::surface.destroy();
    VK::deleteLogicalDevice();
    VK::deleteInstance();
}

void RenderEngine::window_create(int width, int height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    VK::window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    //glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, window_callback_resize);
}

void RenderEngine::window_callback_resize(GLFWwindow* window, int width, int height) {
    RenderEngine::width = width;
    RenderEngine::height = height;
}

void RenderEngine::window_destroy() {
    glfwDestroyWindow(window);
}

void RenderEngine::window_set_fullscreen(bool fullscreen) {

}

void RenderEngine::window_is_fullscreen() {

}

bool RenderEngine::window_is_closed() {
    return glfwWindowShouldClose(window);
}
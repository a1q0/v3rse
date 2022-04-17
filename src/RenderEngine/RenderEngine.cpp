#include "RenderEngine.h"

#include "glfw_vulkan.h"
#include "VK/VK.h"

#include <chrono>

VkInstance instance = nullptr;
VkSurfaceKHR surface = nullptr;
VkPhysicalDevice physicalDevice = nullptr;
VkDevice device = nullptr;
VkSwapchainKHR swapchain = nullptr;

vector<VkFramebuffer> swapchain_framebuffers;
vector<VkImageView> swapchain_images_view;
vector<VkImage> swapchain_images;
VkFormat swapchain_images_format;
VkExtent2D swapchain_images_extent;

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

vector<Vertex> vertices = {{{0.0f,  -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                           {{0.5f,  0.5f,  0.0f}, {0.0f, 1.0f, 0.0f}},
                           {{-0.5f, 0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}}};

void RenderEngine::init() {
    window_create(width, height, "v3rse");
    instance = VK::createInstance();
    VK::createDebugMessenger(instance);
    surface = VK::createSurface(instance, window);
    physicalDevice = VK::getBestPhysicalDevice(instance, surface);
    QueueFamilyIndices queueFamilyIndices = VK::getQueueFamilyIndices(physicalDevice, surface);
    device = VK::createLogicalDevice(physicalDevice, VK::getQueueCreateInfos(queueFamilyIndices));
    SurfaceDetails surfaceDetails = VK::getSurfaceDetails(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = VK::chooseSwapSurfaceFormat(surfaceDetails.vkSurfaceFormats);
    VkPresentModeKHR presentMode = VK::chooseSwapPresentMode(surfaceDetails.vkPresentModes);

    int fb_width;
    int fb_height;

    window_framebuffer_size(fb_width, fb_height);
    info("{} {}", fb_width, fb_height);
    VkExtent2D extent = VK::chooseSwapExtent(surfaceDetails.vkSurfaceCapabilities, width, height);

    swapchain = VK::createSwapchain(physicalDevice, device, surface, surfaceDetails, width, height, surfaceFormat,
                                    presentMode, extent);
    swapchain_images_format = surfaceFormat.format;
    swapchain_images_extent = extent;

    vkGetDeviceQueue(device, queueFamilyIndices.graphics.value(), 0, &queue_graphics);
    vkGetDeviceQueue(device, queueFamilyIndices.present.value(), 0, &queue_present);

    if (queue_graphics == queue_present) info("using same queue for graphics and presentation");

    swapchain_images = VK::getSwapchainImages(device, swapchain);

    swapchain_images_view.resize(swapchain_images.size());
    for (int i = 0; i < swapchain_images.size(); i++) {
        swapchain_images_view[i] = VK::createImageView(device, swapchain_images[i], swapchain_images_format,
                                                       VK_IMAGE_ASPECT_COLOR_BIT);
    }

    renderPass = VK::createRenderPass(device, surfaceFormat.format);
    VK::createGraphicsPipeline(device, extent, renderPass, pipelineLayout, pipeline);

    swapchain_framebuffers.resize(swapchain_images.size());
    for (int i = 0; i < swapchain_images.size(); i++) {
        swapchain_framebuffers[i] = VK::createFramebuffer(device, renderPass, extent.width, extent.height,
                                                          {swapchain_images_view[i]});
    }

    commandPool = VK::createCommandPool(device, queueFamilyIndices);

    VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext{},
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
    };

    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkSemaphoreCreateInfo semaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
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
    vkDeviceWaitIdle(device);
}

void RenderEngine::frame() {
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result_acquireNextImage = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores,
                                                             VK_NULL_HANDLE, &imageIndex);
    if (result_acquireNextImage == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }

    vkResetFences(device, 1, &inFlightFence);

    vkResetCommandBuffer(commandBuffer, 0); /*VkCommandBufferResetFlagBits*/
    VK::recordCommandBuffer(commandBuffer, renderPass, swapchain_framebuffers[imageIndex],
                            swapchain_images_extent, pipeline);

    VkPipelineStageFlags stageFlags[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo{
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

    VkPresentInfoKHR presentInfo{
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

    vkDestroyFence(device, inFlightFence, nullptr);
    vkDestroySemaphore(device, renderFinishedSemaphores, nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphores, nullptr);

    for (const auto& f: swapchain_framebuffers) {
        VK::deleteFramebuffer(device, f);
    }

    for (const auto& iv: swapchain_images_view) {
        VK::deleteImageView(device, iv);
    }

    VK::deleteCommandPool(device, commandPool);
    VK::deleteRenderPass(device, renderPass);
    VK::deletePipelineLayout(device, pipelineLayout);
    VK::deletePipeline(device, pipeline);
    VK::deleteSwapchain(device, swapchain);
    VK::deleteSurface(instance, surface);
    VK::deleteLogicalDevice(device);
    VK::deleteInstance(instance);
}

void RenderEngine::window_create(int width, int height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    //glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, window_callback_resize);
}

void RenderEngine::window_callback_resize(GLFWwindow* window, int width, int height) {
    RenderEngine::width = width;
    RenderEngine::height = height;
}

void RenderEngine::window_destroy() {

}

void RenderEngine::window_set_fullscreen(bool fullscreen) {

}

void RenderEngine::window_is_fullscreen() {

}

bool RenderEngine::window_is_closed() {
    return glfwWindowShouldClose(window);
}
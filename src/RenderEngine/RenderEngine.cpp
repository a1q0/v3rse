#include "RenderEngine.h"

#include "glfw_vulkan.h"
#include "VK/VK.h"

#include <chrono>

VkInstance instance = nullptr;
VkSurfaceKHR surface = nullptr;
VkPhysicalDevice physicalDevice = nullptr;
VkDevice device = nullptr;
VkSwapchainKHR  swapchain = nullptr;
GLFWwindow* window = nullptr;

uint32_t width = 480;
uint32_t height = 480;

void RenderEngine::init() {
    window_create(width, height, "v3rse");
    instance = VK::createInstance();
    VK::createDebugMessenger(instance);
    surface = VK::createSurface(instance, window);
    physicalDevice = VK::getBestPhysicalDevice(instance, surface);
    QueueFamilyIndices queueFamilyIndices = VK::getQueueFamilyIndices(physicalDevice, surface);
    device = VK::createLogicalDevice(physicalDevice, VK::getQueueCreateInfos(queueFamilyIndices));
    swapchain = VK::createSwapchain(physicalDevice, device, surface, width, height);
}

void RenderEngine::loop() {
    while (!window_is_closed()) {
//        typedef std::chrono::high_resolution_clock Clock;
//        auto t1 = Clock::now();
//        auto t2 = Clock::now();

        frame();

        window_update();
    }
//    vkDeviceWaitIdle(vkDevice);
}

void RenderEngine::frame() {

}

void RenderEngine::exit() {

}

void RenderEngine::window_create(int width, int height, const char *title) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    //glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, window_callback_resize);
}

void RenderEngine::window_callback_resize(GLFWwindow* window, int width, int height) {

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
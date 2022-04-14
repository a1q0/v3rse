#include "RenderEngine.h"

#include "glfw_vulkan.h"
#include "VK/VK.h"

#include <chrono>

VkInstance instance = nullptr;
GLFWwindow* window = nullptr;

void RenderEngine::init() {
    window_create(480, 480, "v3rse");
    instance = VK::createInstance();
    VK::createDebugMessenger(instance);
    VK::createSurface(instance, window);
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
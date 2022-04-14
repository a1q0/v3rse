#include "RenderEngine.h"

#include "glfw_vulkan.h"
#include "VK/VK.h"

GLFWwindow *RenderEngine::window = nullptr;

void RenderEngine::init() {
    VK::createInstance("", {""}, true);
    VK::setDebugCallback();
}

void RenderEngine::loop() {
    while (!window_is_closed()) {
        glfwPollEvents();
        frame();
    }

    vkDeviceWaitIdle(vkDevice);
}

void RenderEngine::frame() {

}

void RenderEngine::exit() {

}

void RenderEngine::window_update() {

}

void RenderEngine::window_create(uint32_t width, uint32_t height, const char *title) {

}

void RenderEngine::window_destroy() {

}

void RenderEngine::window_set_fullscreen(bool) {

}

void RenderEngine::window_is_fullscreen() {

}

bool RenderEngine::window_is_closed() {
    return glfwWindowShouldClose(window);
}
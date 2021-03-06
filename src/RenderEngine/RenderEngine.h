#pragma once

#include <cstdint>
#include "glfw_vulkan.h"
#include "using_std.h"
#include "using_glm.h"
#include "logging.h"

#include <glm/glm.hpp>

using glm::vec3;

namespace RenderEngine {
    inline GLFWwindow* window;
    inline int width = 480;
    inline int height = 480;

    // render engine
    void init();

    void loop();

    void frame();

    void exit();

    // window
    void window_create(int width, int height, const char* title);

    void window_callback_resize(GLFWwindow* window, int width, int height);

    inline void window_update() { glfwPollEvents(); }

    inline void window_framebuffer_size(int& width, int& height) {
        return glfwGetFramebufferSize(window, &width, &height);
    }

    void window_destroy();

    bool window_is_closed();

    void window_set_fullscreen(bool);

    void window_is_fullscreen();
}

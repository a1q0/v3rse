#pragma once

#include <stdint.h>
#include "glfw_vulkan.h"

namespace RenderEngine {
    // render engine
	void init();
    void loop();
	void frame();
	void exit();
	// window
    void window_create(int width, int height, const char *title);
    void window_callback_resize(GLFWwindow *window, int width, int height);
    inline void window_update() { glfwPollEvents(); }
    void window_destroy();
    bool window_is_closed();
    void window_set_fullscreen(bool);
    void window_is_fullscreen();
};


#pragma once

#include <stdint.h>
#include "glfw_vulkan.h"

class RenderEngine {
public:
	// render engine
	void init();
	void frame();
	void exit();
	// window
    void window_create(uint32_t width, uint32_t height, const char* title);
    void window_update();
    void window_destroy();
    bool window_is_closed();
    void window_set_fullscreen(bool);
    void window_is_fullscreen();

    static GLFWwindow *window;

    void loop();
};


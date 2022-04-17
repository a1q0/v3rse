#pragma once

#include <cstdint>
#include "glfw_vulkan.h"
#include "../using_std.h"
#include "../using_glm.h"

#include <glm/glm.hpp>

using glm::vec3;

struct Vertex {
    vec3 pos;
    vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        return VkVertexInputBindingDescription {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
    }

    static array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions(uint32_t binding_id) {
        return array<VkVertexInputAttributeDescription, 2> {
            VkVertexInputAttributeDescription {
                .location = 0,
                .binding = binding_id,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = static_cast<uint32_t>(offsetof(Vertex, pos))
            },
            VkVertexInputAttributeDescription {
                .location = 1,
                .binding = binding_id,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = static_cast<uint32_t>(offsetof(Vertex, pos))
            }
        };
    }

    static VkBuffer createVertexBuffer(VkDevice vkDevice, const vector<Vertex>& vertices,
                                       VkBufferUsageFlags vkBufferUsageFlags,
                                       VkSharingMode vkSharingMode) {
        VkBufferCreateInfo vkBufferCreateInfo {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext{},
            .flags{},
            .size = sizeof(vertices[0]) * vertices.size(),
            .usage = vkBufferUsageFlags,
            .sharingMode = vkSharingMode,
            .queueFamilyIndexCount{}, // TODO: maybe this has to be set to the current queue ?
            .pQueueFamilyIndices{}
        };

        VkBuffer vkBuffer;
        vkCreateBuffer(vkDevice, &vkBufferCreateInfo, nullptr, &vkBuffer);

        VkMemoryRequirements vkMemoryRequirements;
        vkGetBufferMemoryRequirements(vkDevice, vkBuffer, &vkMemoryRequirements);

        return vkBuffer;
    }
};

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

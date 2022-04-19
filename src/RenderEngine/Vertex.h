#pragma once

#include "glfw_vulkan.h"
#include "using_std.h"
#include "using_glm.h"
#include "logging.h"

class Vertex {
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
                                       VkSharingMode vkSharingMode);
};



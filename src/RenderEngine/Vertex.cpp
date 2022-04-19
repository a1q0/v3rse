#include "Vertex.h"

//#include "VK/VK.h"
//
//uint32_t availableMemory = 0;
//
//VkBuffer Vertex::createVertexBuffer(VkDevice vkDevice, const vector <Vertex>& vertices, VkSharingMode vkSharingMode) {
//    VkBufferCreateInfo vkBufferCreateInfo {
//        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
//        .pNext{},
//        .flags{},
//        .size = sizeof(vertices[0]) * vertices.size(),
//        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//        .sharingMode = vkSharingMode,
//        .queueFamilyIndexCount{}, // pQueueFamilyIndices is a pointer to an array of queue families that will access
//        // this buffer. It is ignored if sharingMode is not VK_SHARING_MODE_CONCURRENT.
//        .pQueueFamilyIndices{}
//    };
//
//    VkBuffer vkBuffer;
//    vkCreateBuffer(vkDevice, &vkBufferCreateInfo, nullptr, &vkBuffer);
//
//    VkMemoryRequirements vkMemoryRequirements;
//    vkGetBufferMemoryRequirements(vkDevice, vkBuffer, &vkMemoryRequirements);
//
//    info("memory requirements:\nsize: {}\nalignment: {}", vkMemoryRequirements.size,
//         vkMemoryRequirements.alignment);
//
//    VkMemoryAllocateInfo allocInfo {
//        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
//        .pNext{},
//        .allocationSize = vkMemoryRequirements.size,
//        .memoryTypeIndex = VK::findMemoryTypes(vkMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)[0]
//    };
//    return vkBuffer;
//}
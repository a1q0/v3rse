#pragma once

#include "glfw_vulkan.h"
#include "logging.h"

typedef VkPhysicalDeviceMemoryProperties VkMemoryProperties;

namespace VK {

    // - VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT bit specifies that memory allocated with this type is the most efficient
    // for device access. This property will be set if and only if the memory type belongs
    // to a heap with the VK_MEMORY_HEAP_DEVICE_LOCAL_BIT set.
    //
    // - VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT bit specifies that memory allocated with this type
    // can be mapped for host access using vkMapMemory.
    //
    // - VK_MEMORY_PROPERTY_HOST_COHERENT_BIT bit specifies that the host cache management commands
    // vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges are not needed to flush host writes
    // to the device or make device writes visible to the host, respectively.
    //
    // - VK_MEMORY_PROPERTY_HOST_CACHED_BIT bit specifies that memory allocated with this type is cached on the host.
    // Host memory accesses to uncached memory are slower than to cached memory,
    // however uncached memory is always host coherent.
    //
    // - VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT bit specifies that the memory type only allows device access to the memory.
    // Memory types must not have both VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT and VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT set.
    // Additionally, the object’s backing memory may be provided by the implementation lazily as specified
    // in Lazily Allocated Memory.
    //
    // - VK_MEMORY_PROPERTY_PROTECTED_BIT bit specifies that the memory type only allows device access to the memory,
    // and allows protected queue operations to access the memory. Memory types must not
    // have VK_MEMORY_PROPERTY_PROTECTED_BIT set and any of VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT set,
    // or VK_MEMORY_PROPERTY_HOST_COHERENT_BIT set, or VK_MEMORY_PROPERTY_HOST_CACHED_BIT set.
    //
    // - VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD bit specifies that device accesses to allocations of this memory type
    // are automatically made available and visible.
    //
    // - VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD bit specifies that memory allocated with this type
    // is not cached on the device. Uncached device memory is always device coherent.
    //
    // - VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV bit specifies that external devices can access this memory directly.
    //
    // For any memory allocated with both the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT and
    // the VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD, host or device accesses also perform automatic
    // memory domain transfer operations, such that writes are always automatically available
    // and visible to both host and device memory domains.
    _inline VkMemoryProperties getPhysicalDeviceMemoryProperties(VkPhysicalDevice vkPhysicalDevice) {
        VkMemoryProperties vkMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vkMemoryProperties);

        for (int i = 0; i < vkMemoryProperties.memoryHeapCount; i++) {
            info("i: {} -> size: {} bytes, flags: {} ", i, vkMemoryProperties.memoryHeaps[i].size / 2e,
                 vkMemoryProperties.memoryHeaps[i].flags);
        }

        for (int i = 0; i < vkMemoryProperties.memoryTypeCount; i++) {
            //info("{} -> {} heap index", i, vkPhysicalDeviceMemoryProperties.memoryTypes->heapIndex);
            info("i: {} -> heap: {}", i, vkMemoryProperties.memoryTypes[i].heapIndex);

            if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                info("VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT");
            if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                info("VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT");
            if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                info("VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ");
            if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
                info("VK_MEMORY_PROPERTY_HOST_CACHED_BIT");
            if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
                info("VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT");
            if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
                info("VK_MEMORY_PROPERTY_PROTECTED_BIT");
            if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)
                info("VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD");
            if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD)
                info("VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD");
            if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV)
                info("VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV");
        }

        return vkMemoryProperties;
    }

    _inline vector<uint32_t> findMemoryTypes(VkPhysicalDevice vkPhysicalDevice, uint32_t type, uint32_t flags) {
        VkMemoryProperties vkMemoryProperties = getPhysicalDeviceMemoryProperties(vkPhysicalDevice);
        vector<uint32_t> types;

        for (int i = 0; i < vkMemoryProperties.memoryTypeCount; i++) {
            if ((1 << i) & type && (vkMemoryProperties.memoryHeaps->flags & flags) == flags) types.push_back(i);
        }

        return types;
    }
}
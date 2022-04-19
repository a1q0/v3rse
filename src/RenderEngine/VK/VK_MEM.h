#pragma once

#include "glfw_vulkan.h"
#include "logging.h"

#include <cmath>

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
    force_inline VkMemoryProperties getPhysicalDeviceMemoryProperties(VkPhysicalDevice vkPhysicalDevice) {
        VkMemoryProperties vkMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vkMemoryProperties);

        // - VK_MEMORY_HEAP_DEVICE_LOCAL_BIT specifies that the heap corresponds to device-local memory.
        // Device-local memory may have different performance characteristics than host-local memory,
        // and may support different memory property flags.
        // - VK_MEMORY_HEAP_MULTI_INSTANCE_BIT specifies that in a logical device representing more
        // than one physical device, there is a per-physical device instance of the heap memory.
        // By default, an allocation from such a heap will be replicated to each physical device’s instance of the heap.

        //for (int i = 0; i < vkMemoryProperties.memoryHeapCount; i++) {
        //    info("heap: {} -> size: {} MiB, flags: {} ", i, (vkMemoryProperties.memoryHeaps[i].size / pow(1024, 2)),
        //         vkMemoryProperties.memoryHeaps[i].flags);
        //}
        //
        //for (int i = 0; i < vkMemoryProperties.memoryTypeCount; i++) {
        //    info("i: {} -> heap: {}", i, vkMemoryProperties.memoryTypes[i].heapIndex);
        //
        //    if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        //        info("VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT");
        //    if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        //        info("VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT");
        //    if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        //        info("VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ");
        //    if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
        //        info("VK_MEMORY_PROPERTY_HOST_CACHED_BIT");
        //    if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
        //        info("VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT");
        //    if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
        //        info("VK_MEMORY_PROPERTY_PROTECTED_BIT");
        //    if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)
        //        info("VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD");
        //    if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD)
        //        info("VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD");
        //    if (vkMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV)
        //        info("VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV");
        //}

        return vkMemoryProperties;
    }

    force_inline vector<uint32_t>
    findMemoryTypes(uint32_t type, uint32_t propertyFlags, VkPhysicalDevice vkPhysicalDevice = VK::physicalDevice) {
        VkMemoryProperties vkMemoryProperties = getPhysicalDeviceMemoryProperties(vkPhysicalDevice);
        vector<uint32_t> types;

        for (int i = 0; i < vkMemoryProperties.memoryTypeCount; i++) {
            if ((1 << i) & type && (vkMemoryProperties.memoryTypes->propertyFlags & propertyFlags) == propertyFlags)
                types.push_back(i);
        }

        return types;
    }


}
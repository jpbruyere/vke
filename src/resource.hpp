#pragma once

#include "vke.h"

namespace vke {
    struct Resource
    {
        device_t*               device;
        VkDeviceMemory          deviceMemory= VK_NULL_HANDLE;
        VkMemoryRequirements    memReqs    = {};

        virtual VkWriteDescriptorSet getWriteDescriptorSet(VkDescriptorSet ds, uint32_t binding, VkDescriptorType descriptorType) = 0;
    };
}

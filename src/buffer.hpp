/*
* Vulkan buffer class
*
* Encapsulates a Vulkan buffer
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include "vke.hpp"
#include "resource.hpp"

namespace vke
{
    struct buffer_t : public Resource
    {
        VkBuffer            buffer = VK_NULL_HANDLE;

        VkBufferCreateInfo  infos = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};

        VkDescriptorBufferInfo descriptor;
        VkDeviceSize size = 0;
        VkDeviceSize alignment = 0;
        VkBufferUsageFlags usageFlags;
        VkMemoryPropertyFlags memoryPropertyFlags;
        void* mapped = nullptr;

        buffer_t(device_t* _device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags _memoryPropertyFlags, VkDeviceSize size, void *data = nullptr);
        ~buffer_t();

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();

        VkResult bind(VkDeviceSize offset = 0);

        void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        virtual VkWriteDescriptorSet getWriteDescriptorSet(VkDescriptorSet ds, uint32_t binding, VkDescriptorType descriptorType);

        void copyTo(void* data, VkDeviceSize size);

        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void destroy();

    };
}

/*
* Vulkan device class
*
* Encapsulates a physical Vulkan device and it's logical representation
*
* Copyright (C) 2016-2018 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include "vke.hpp"
#include "phyInfos.hpp"

namespace vke
{
    struct device_t
    {
        VkPhysicalDevice            phy;
        VkDevice                    dev;
        VkPhysicalDeviceProperties  properties;
        VkPhysicalDeviceFeatures    features;
        VkPhysicalDeviceFeatures    enabledFeatures;
        VkPhysicalDeviceMemoryProperties    memoryProperties;
        std::vector<VkQueueFamilyProperties>queueFamilyProperties;

        VkCommandPool   commandPool     = VK_NULL_HANDLE;
        VkPipelineCache pipelineCache   = VK_NULL_HANDLE;
        bool            savePLCache     = true;

        VkQueue queue;

        struct {
            uint32_t graphics;
            uint32_t compute;
        } queueFamilyIndices;

        device_t(vkPhyInfo* phyInfos, const std::vector<const char *> &devLayers);
        ~device_t();

        uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound = nullptr);
        VkFormat getSuitableDepthFormat ();
        uint32_t getQueueFamilyIndex(VkQueueFlagBits queueFlags);

        VkCommandPool createCommandPool(uint32_t queueFamilyIndex,
                                        VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);

        void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
        VkSemaphore createSemaphore ();
        VkFence createFence (bool signaled = false);
        void destroyFence (VkFence fence);
        void destroySemaphore (VkSemaphore semaphore);
        VkPipelineShaderStageCreateInfo loadShader(std::string filename, VkShaderStageFlagBits stage);
    };
}

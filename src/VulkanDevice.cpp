#include "VulkanDevice.hpp"
#include "macros.hpp"
namespace vke {

    device_t::device_t(vkPhyInfo *phyInfos, const std::vector<const char*>& devLayers)
    {
        phy = phyInfos->phy;

        properties			= phyInfos->properties;
        memoryProperties	= phyInfos->memProps;
        features			= phyInfos->features;

        //affect priorities to qcis from vector
        for (uint i=0; i<phyInfos->pQueueInfos.size(); i++)
            phyInfos->pQueueInfos[i].pQueuePriorities = phyInfos->qPriorities[i].data();

        std::vector<const char*> devExtentions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkDeviceCreateInfo devInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        devInfo.queueCreateInfoCount    = (uint32_t)phyInfos->pQueueInfos.size();
        devInfo.pQueueCreateInfos       = phyInfos->pQueueInfos.data();
        devInfo.enabledExtensionCount   = (uint32_t)devExtentions.size();
        devInfo.ppEnabledExtensionNames = devExtentions.data();
        devInfo.enabledLayerCount       = (uint32_t)devLayers.size();
        devInfo.ppEnabledLayerNames     = devLayers.data();
        devInfo.pEnabledFeatures        = &enabledFeatures;


        VK_CHECK_RESULT(vkCreateDevice (phy, &devInfo, nullptr, &dev));

        commandPool = createCommandPool (phyInfos->gQueues[0]);

        vkGetDeviceQueue(dev, phyInfos->gQueues[0], 0, &queue);

        //reload pipeline cache if it exists
        std::ifstream is ("pipeline.cache", std::ifstream::binary);
        char* buffer = nullptr;
        int length = 0;
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
        if (is) {
            is.seekg (0, is.end);
            length = is.tellg();
            if (length > 0) {
                is.seekg (0, is.beg);
                buffer = new char [length];
                is.read (buffer,length);
            }
            is.close();
            pipelineCacheCreateInfo.initialDataSize = length;
            pipelineCacheCreateInfo.pInitialData = buffer;
        }

        VK_CHECK_RESULT(vkCreatePipelineCache (dev, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
        delete[] buffer;
    }

    vke::device_t::~device_t()
    {
        if (pipelineCache){
            if (savePLCache){
                std::ofstream os("pipeline.cache", std::ofstream::binary);
                size_t plCacheSize = 0;
                char* plCache = nullptr;
                VK_CHECK_RESULT(vkGetPipelineCacheData(dev, pipelineCache, &plCacheSize, nullptr));
                plCache = new char [plCacheSize];
                VK_CHECK_RESULT(vkGetPipelineCacheData(dev, pipelineCache, &plCacheSize, plCache));
                os.write (plCache, plCacheSize);
                os.close();
                delete[] plCache;
            }
            vkDestroyPipelineCache(dev, pipelineCache, nullptr);
        }
        if (commandPool)
            vkDestroyCommandPool(dev, commandPool, nullptr);
        if (dev)
            vkDestroyDevice(dev, nullptr);
    }

    uint32_t vke::device_t::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound)
    {
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((typeBits & 1) == 1) {
                if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    if (memTypeFound) {
                        *memTypeFound = true;
                    }
                    return i;
                }
            }
            typeBits >>= 1;
        }

        if (memTypeFound) {
            *memTypeFound = false;
            return 0;
        } else {
            throw std::runtime_error("Could not find a matching memory type");
        }
    }

    VkFormat vke::device_t::getSuitableDepthFormat () {
        std::vector<VkFormat> depthFormats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
        for (auto& format : depthFormats) {
            VkFormatProperties formatProps;
            vkGetPhysicalDeviceFormatProperties(phy, format, &formatProps);
            if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                return format;
        }
        std::cerr << "No suitable depth format found" << std::endl;
        exit(-1);
    }

    uint32_t vke::device_t::getQueueFamilyIndex(VkQueueFlagBits queueFlags)
    {
        // Dedicated queue for compute
        // Try to find a queue family index that supports compute but not graphics
        if (queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
                if ((queueFamilyProperties[i].queueFlags & queueFlags) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                    return i;
                    break;
                }
            }
        }

        // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
            if (queueFamilyProperties[i].queueFlags & queueFlags) {
                return i;
                break;
            }
        }

        throw std::runtime_error("Could not find a matching queue family index");
    }


    VkCommandPool vke::device_t::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
        cmdPoolInfo.flags = createFlags;
        VkCommandPool cmdPool;
        VK_CHECK_RESULT(vkCreateCommandPool(dev, &cmdPoolInfo, nullptr, &cmdPool));
        return cmdPool;
    }

    VkCommandBuffer vke::device_t::createCommandBuffer(VkCommandBufferLevel level, bool begin)
    {
        VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool = commandPool;
        cmdBufAllocateInfo.level = level;
        cmdBufAllocateInfo.commandBufferCount = 1;

        VkCommandBuffer cmdBuffer;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(dev, &cmdBufAllocateInfo, &cmdBuffer));

        // If requested, also start recording for the new command buffer
        if (begin) {
            VkCommandBufferBeginInfo cmdBufInfo{};
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
        }

        return cmdBuffer;
    }

    void vke::device_t::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
    {
        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence;
        VK_CHECK_RESULT(vkCreateFence(dev, &fenceInfo, nullptr, &fence));

        // Submit to the queue
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
        // Wait for the fence to signal that command buffer has finished executing
        VK_CHECK_RESULT(vkWaitForFences(dev, 1, &fence, VK_TRUE, 100000000000));

        vkDestroyFence(dev, fence, nullptr);

        if (free) {
            vkFreeCommandBuffers(dev, commandPool, 1, &commandBuffer);
        }
    }
    VkSemaphore vke::device_t::createSemaphore ()
    {
        VkSemaphore sema = VK_NULL_HANDLE;
        VkSemaphoreCreateInfo semaphoreCreateInfo {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VK_CHECK_RESULT(vkCreateSemaphore (dev, &semaphoreCreateInfo, nullptr, &sema));
        return sema;
    }
    VkFence vke::device_t::createFence (bool signaled) {
        VkFence fence = VK_NULL_HANDLE;
        VkFenceCreateInfo fenceCreateInfo {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, VK_NULL_HANDLE,
                    signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0};
        VK_CHECK_RESULT(vkCreateFence (dev, &fenceCreateInfo, nullptr, &fence));
        return fence;
    }
    void device_t::destroyFence (VkFence fence) {
        vkDestroyFence (dev, fence, VK_NULL_HANDLE);
    }
    void device_t::destroySemaphore (VkSemaphore semaphore) {
        vkDestroySemaphore (dev, semaphore, VK_NULL_HANDLE);
    }
    VkPipelineShaderStageCreateInfo device_t::loadShader(std::string filename, VkShaderStageFlagBits stage)
    {
        VkPipelineShaderStageCreateInfo shaderStage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        shaderStage.stage = stage;
        shaderStage.pName = "main";
        std::ifstream is("shaders/" + filename, std::ios::binary | std::ios::in | std::ios::ate);

        if (is.is_open()) {
            size_t size = is.tellg();
            is.seekg(0, std::ios::beg);
            char* shaderCode = new char[size];
            is.read(shaderCode, size);
            is.close();
            assert(size > 0);
            VkShaderModuleCreateInfo moduleCreateInfo{};
            moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            moduleCreateInfo.codeSize = size;
            moduleCreateInfo.pCode = (uint32_t*)shaderCode;
            vkCreateShaderModule(dev, &moduleCreateInfo, NULL, &shaderStage.module);
            delete[] shaderCode;
        }
        else {
            std::cerr << "Error: Could not open shader file \"" << filename << "\"" << std::endl;
            shaderStage.module = VK_NULL_HANDLE;
        }
        assert(shaderStage.module != VK_NULL_HANDLE);
        return shaderStage;
    }
}

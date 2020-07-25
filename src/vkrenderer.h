#pragma once

#include "VkEngine.h"
#include "VulkanSwapChain.hpp"

#define DRAW_FENCE_TIMEOUT 99900000

namespace vke {

    class vkRenderer
    {
    protected:

        bool prepared = false;

        vke::VulkanDevice*          device;
        vke::RenderTarget*          renderTarget;
        vke::VkEngine::UniformBuffers sharedUBOs;

        vke::ShadingContext*        shadingCtx;

        VkCommandPool               commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer>cmdBuffers;

        VkDescriptorSet         descriptorSet   = VK_NULL_HANDLE;

        std::vector<VkFence>    fences;
        VkSubmitInfo            submitInfo;

        VkPipeline              pipeline        = VK_NULL_HANDLE;
        VkPipelineLayout        pipelineLayout  = VK_NULL_HANDLE;

        std::vector<float>	vertices;
        vke::Buffer			vertexBuff;
        uint32_t			vBufferSize = 10000 * sizeof(float) * 6;

        virtual void prepare();
        virtual void prepareRendering();
        virtual void configurePipelineLayout();
        virtual void loadRessources();
        virtual void freeRessources();
        virtual void prepareDescriptors();
        virtual void preparePipeline();

    public:
        VkSemaphore         drawComplete    = VK_NULL_HANDLE;
        uint32_t			vertexCount = 0;
        uint32_t			sdffVertexCount = 0;

        vkRenderer ();
        virtual ~vkRenderer();

        virtual void create (ptrVkDev _device, vke::RenderTarget* _renderTarget,
                                               VkEngine::UniformBuffers& _sharedUbos);
        virtual void destroy();

        virtual void buildCommandBuffer ();
        virtual void rebuildCommandBuffer ();
        virtual void draw(VkCommandBuffer cmdBuff);

        void submit (VkQueue queue, VkSemaphore *waitSemaphore, uint32_t waitSemaphoreCount);

        void drawLine(const glm::vec3& from,const glm::vec3& to,const glm::vec3& fromColor, const glm::vec3& toColor);
        void drawLine(const glm::vec3& from,const glm::vec3& to,const glm::vec3& color);

        virtual void flush();
        virtual void clear();
    };
}


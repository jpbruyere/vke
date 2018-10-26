/*
* Class wrapping access to the swap chain
*
* A swap chain is a collection of framebuffers used for rendering and presentation to the windowing system
*
* Copyright (C) 2016-2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include "vke.hpp"

namespace vke {

    typedef struct _SwapChainBuffers {
        VkImage image;
        VkImageView view;
    } SwapChainBuffer;

    class swap_chain_t
    {
    private:
        // Function pointers
        PFN_vkGetPhysicalDeviceSurfaceSupportKHR        fpGetPhysicalDeviceSurfaceSupportKHR;
        PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR   fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
        PFN_vkGetPhysicalDeviceSurfaceFormatsKHR        fpGetPhysicalDeviceSurfaceFormatsKHR;
        PFN_vkGetPhysicalDeviceSurfacePresentModesKHR   fpGetPhysicalDeviceSurfacePresentModesKHR;
        PFN_vkCreateSwapchainKHR    fpCreateSwapchainKHR;
        PFN_vkDestroySwapchainKHR   fpDestroySwapchainKHR;
        PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
        PFN_vkAcquireNextImageKHR   fpAcquireNextImageKHR;
        PFN_vkQueuePresentKHR       fpQueuePresentKHR;
    public:
        engine_t*                   vke;
        VkSwapchainCreateInfoKHR    infos = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};

        VkSwapchainKHR              swapChain = VK_NULL_HANDLE;

        uint32_t                    imageCount;
        std::vector<SwapChainBuffer>buffers;
        uint32_t                    currentBuffer = 0;
        VkSemaphore                 presentCompleteSemaphore;
        VkPresentInfoKHR            presentInfo;
        VkFormat                    depthFormat;

        std::vector<render_target_t*>  boundRenderTargets;

        //RenderTarget* multisampleTarget;
        Texture* depthStencil;

        swap_chain_t(engine_t* _vke, bool vsync = true);
        virtual ~swap_chain_t();

        void create(uint32_t& width, uint32_t& height);

        inline VkResult acquireNextImage(VkDevice dev) {
            return fpAcquireNextImageKHR(dev, swapChain, UINT64_MAX,
                                         presentCompleteSemaphore, (VkFence)nullptr, &currentBuffer);
        }
        inline VkResult queuePresent(VkQueue queue, VkSemaphore waitSemaphore = VK_NULL_HANDLE) {
            presentInfo.pImageIndices = &currentBuffer;
            presentInfo.pWaitSemaphores = &waitSemaphore;

            if (waitSemaphore == VK_NULL_HANDLE)
                presentInfo.waitSemaphoreCount = 0;
            else
                presentInfo.waitSemaphoreCount = 1;

            return fpQueuePresentKHR(queue, &presentInfo);
        }
    };

}

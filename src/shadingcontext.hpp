#pragma once

#include "vke.hpp"

namespace vke {
    struct BindingSlot {
        uint layoutIdx;
        uint descriptorIdx;
        Resource* pResource;
    };

    //should be thread safe with dedicate desc and cmd pools
    class ShadingContext
    {
        device_t*               			device;
        VkDescriptorPool					descriptorPool;
        VkCommandPool                       cmdPool;//?


        std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindings;

        uint32_t                maxSets;

    public:
        std::vector<VkDescriptorSetLayout>	layouts;

        ShadingContext(device_t* _device, uint32_t maxDescriptorSet = 1);
        virtual ~ShadingContext();

        uint32_t addDescriptorSetLayout (const std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBinding);

        void prepare ();

        VkDescriptorSet allocateDescriptorSet (uint32_t layoutIndex);
        void updateDescriptorSet (VkDescriptorSet ds, const std::vector<BindingSlot>& slots);
    };
}

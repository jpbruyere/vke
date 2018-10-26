/*
* Vulkan texture loader
*
* Copyright(C) 2016-2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license(MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include "vke.hpp"

#include <gli/gli.hpp>

#include "resource.hpp"
#include "buffer.hpp"
#include "VulkanDevice.hpp"

namespace vke
{
    struct Texture : public Resource {
        VkImage             image       = VK_NULL_HANDLE;
        VkImageView         view        = VK_NULL_HANDLE;
        VkSampler           sampler     = VK_NULL_HANDLE;

        VkImageCreateInfo   infos       = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        VkImageLayout       imageLayout;
        VkDescriptorImageInfo descriptor;

        void create (device_t* _device,
                        VkImageType imageType, VkFormat format, uint32_t width, uint32_t height,
                        VkImageUsageFlags usage, VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                        uint32_t  mipLevels = 1, uint32_t arrayLayers = 1,
                        VkImageCreateFlags flags = 0,
                        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
                        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);
        Texture ();
        Texture (device_t*  _device, VkFormat _format, VkImage importedImg = VK_NULL_HANDLE,
                 uint32_t _width = 0, uint32_t height = 0, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
                 uint32_t  mipLevels = 1, uint32_t arrayLayers = 1);
        Texture (device_t*  _device,
                        VkImageType imageType, VkFormat format, uint32_t width, uint32_t height,
                        VkImageUsageFlags usage, VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                        uint32_t  mipLevels = 1, uint32_t arrayLayers = 1,
                        VkImageCreateFlags flags = 0,
                        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
                        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);
        Texture (device_t*  _device,  VkQueue copyQueue, VkImageType imageType, VkFormat format,
                 std::vector<Texture> texArray, uint32_t width, uint32_t height,
                 VkImageUsageFlags _usage = VK_IMAGE_USAGE_SAMPLED_BIT);

        /** @brief Update image descriptor from current sampler, view and image layout */
        void updateDescriptor();

        virtual VkWriteDescriptorSet getWriteDescriptorSet (VkDescriptorSet ds, uint32_t binding, VkDescriptorType descriptorType);

        void destroy();
        void setImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkImageSubresourceRange subresourceRange,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask);

        // Fixed sub resource on first mip level and layer
        void setImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImageAspectFlags aspectMask,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask);

        void copyTo (VkQueue copyQueue, unsigned char* buffer, VkDeviceSize bufferSize);
        void createView (VkImageViewType viewType, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                         uint32_t levelCount = 1, uint32_t layerCount = 1, VkComponentMapping components =
                         {VK_COMPONENT_SWIZZLE_R,VK_COMPONENT_SWIZZLE_G,VK_COMPONENT_SWIZZLE_B,VK_COMPONENT_SWIZZLE_A });

        void createSampler (VkFilter filter = VK_FILTER_NEAREST, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                            VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                            VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
                            VkCompareOp compareOp = VK_COMPARE_OP_NEVER);
        void buildMipmaps (VkQueue copyQueue, VkCommandBuffer _blitCmd = VK_NULL_HANDLE);
        void loadStbLinearNoSampling (std::string filename,
            device_t* device,
            VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            bool flipY = true);
    };
    class Texture2D : public Texture {
    public:
        void loadFromFile(
            std::string filename,
            VkFormat format,
            vke::device_t *_device,
            VkQueue copyQueue,
            VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
#if defined(__ANDROID__)
            // Textures are stored inside the apk on Android (compressed)
            // So they need to be loaded via the asset manager
            AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING);
            if (!asset) {
                LOGE("Could not load texture %s", filename.c_str());
                exit(-1);
            }
            size_t size = AAsset_getLength(asset);
            assert(size > 0);

            void *textureData = malloc(size);
            AAsset_read(asset, textureData, size);
            AAsset_close(asset);

            gli::texture2d tex2D(gli::load((const char*)textureData, size));

            free(textureData);
#else
            gli::texture2d tex2D(gli::load(filename.c_str()));
#endif
            assert(!tex2D.empty());

            create(_device, VK_IMAGE_TYPE_2D, format,
                   static_cast<uint32_t>(tex2D[0].extent().x),static_cast<uint32_t>(tex2D[0].extent().y),
                   imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    VK_IMAGE_TILING_OPTIMAL,static_cast<uint32_t>(tex2D.levels()));

            copyTo(copyQueue, (unsigned char*)tex2D.data(), tex2D.size());

            createView(VK_IMAGE_VIEW_TYPE_2D);
            createSampler(VK_FILTER_LINEAR,VK_SAMPLER_ADDRESS_MODE_REPEAT,VK_SAMPLER_MIPMAP_MODE_LINEAR);

            updateDescriptor();
        }
    };

    class TextureCubeMap : public Texture {
    public:
        void buildFromImages(const std::vector<std::string>& mapDic, uint32_t textureSize,
                             VkFormat _format,
                             vke::device_t *_device,
                             VkQueue copyQueue,
                             VkImageUsageFlags _imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
                             VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){

            create(_device, VK_IMAGE_TYPE_2D, _format,
                   textureSize, textureSize,
                   _imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT| VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_TILING_OPTIMAL,
                   (uint32_t)floor(log2(textureSize)) + 1, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

            imageLayout = _imageLayout;

            for (int l = 0; l < mapDic.size(); l++) {
                Texture inTex;
                inTex.loadStbLinearNoSampling(mapDic[l].c_str(), device);

                VkCommandBuffer blitFirstMipCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

                VkImageBlit firstMipBlit{};
                firstMipBlit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, l, 1};
                firstMipBlit.srcOffsets[1] = {inTex.infos.extent.width,inTex.infos.extent.height,1};
                firstMipBlit.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, l, 1};
                firstMipBlit.dstOffsets[1] = {infos.extent.width,infos.extent.height,1};

                VkImageSubresourceRange firstMipSubRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, l, 1};

                setImageLayout(blitFirstMipCmd,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    firstMipSubRange,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

                // Blit from source texture
                vkCmdBlitImage(blitFirstMipCmd, inTex.image,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &firstMipBlit, VK_FILTER_CUBIC_IMG);

                setImageLayout(blitFirstMipCmd,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    firstMipSubRange,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

                device->flushCommandBuffer(blitFirstMipCmd, copyQueue, true);

                inTex.destroy();

                VkCommandBuffer blitCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

                // mipmap generation
                // Copy down mips from n-1 to n
                for (int32_t i = 1; i < infos.mipLevels; i++)
                {
                    VkImageBlit imageBlit{};
                    imageBlit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, i-1, l, 1};
                    imageBlit.srcOffsets[1] = {infos.extent.width >> (i - 1),infos.extent.height >> (i - 1),1};
                    imageBlit.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, i, l, 1};
                    imageBlit.dstOffsets[1] = {infos.extent.width >> i,infos.extent.height >> i, 1};

                    VkImageSubresourceRange mipSubRange = {VK_IMAGE_ASPECT_COLOR_BIT, i, 1, l, 1};

                    setImageLayout(blitCmd,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        mipSubRange,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

                    // Blit from previous level
                    vkCmdBlitImage(blitCmd,
                        image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1, &imageBlit, VK_FILTER_LINEAR);

                    setImageLayout(blitCmd,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        mipSubRange,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
                }
                firstMipSubRange.levelCount = infos.mipLevels;
                setImageLayout(blitCmd,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageLayout,
                    firstMipSubRange,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
                device->flushCommandBuffer(blitCmd, copyQueue, true);
            }

            createView(VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT,infos.mipLevels,6);
            createSampler(VK_FILTER_CUBIC_IMG,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,VK_SAMPLER_MIPMAP_MODE_LINEAR);
            //samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;

            updateDescriptor();

        }
        void loadFromFile(
            std::string filename,
            VkFormat format,
            vke::device_t *_device,
            VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
            VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
#if defined(__ANDROID__)
            // Textures are stored inside the apk on Android (compressed)
            // So they need to be loaded via the asset manager
            AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING);
            if (!asset) {
                LOGE("Could not load texture %s", filename.c_str());
                exit(-1);
            }
            size_t size = AAsset_getLength(asset);
            assert(size > 0);

            void *textureData = malloc(size);
            AAsset_read(asset, textureData, size);
            AAsset_close(asset);

            gli::texture_cube texCube(gli::load((const char*)textureData, size));

            free(textureData);
#else
            gli::texture_cube texCube(gli::load(filename));
#endif
            assert(!texCube.empty());

            create(_device, VK_IMAGE_TYPE_2D, format,
                   static_cast<uint32_t>(texCube.extent().x),static_cast<uint32_t>(texCube[0].extent().y),
                   imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    VK_IMAGE_TILING_OPTIMAL,static_cast<uint32_t>(texCube.levels()), 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

            imageLayout = _imageLayout;

            vke::buffer_t stagingBuffer (device,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                texCube.size(), texCube.data());

            descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

            size_t offset = 0;

            std::vector<VkBufferImageCopy> bufferCopyRegions;
            for (uint32_t face = 0; face < 6; face++) {
                for (uint32_t level = 0; level < infos.mipLevels; level++) {
                    VkBufferImageCopy bufferCopyRegion = {};
                    bufferCopyRegion.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, level, face, 1};
                    bufferCopyRegion.imageExtent = {(uint32_t)texCube[face][level].extent().x,(uint32_t)texCube[face][level].extent().y,1};
                    bufferCopyRegion.bufferOffset = offset;

                    bufferCopyRegions.push_back(bufferCopyRegion);
                    offset += texCube[face][level].size();
                }
            }
            VkImageSubresourceRange subresourceRange = {};
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = infos.mipLevels;
            subresourceRange.layerCount = 6;

            setImageLayout (copyCmd,
                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           subresourceRange,
                           VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

            vkCmdCopyBufferToImage(copyCmd, stagingBuffer.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

            setImageLayout(copyCmd,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageLayout,
                           subresourceRange,
                           VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

            device->flushCommandBuffer(copyCmd, device->queue, true);

            createView(VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT,infos.mipLevels,6);
            createSampler(VK_FILTER_LINEAR,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,VK_SAMPLER_MIPMAP_MODE_LINEAR);

            updateDescriptor();
        }
    };

}

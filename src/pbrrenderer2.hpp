#pragma once

#include "vke.hpp"
#include "vkrenderer.hpp"
#include "texture.hpp"
#include "VulkanglTFModel.hpp"


class pbrRenderer : public vke::vkRenderer
{
    void generateBRDFLUT();
    void generateCubemaps();
protected:
    virtual void configurePipelineLayout();
    virtual void loadRessources();
    virtual void freeRessources();
    virtual void prepareDescriptors();
    virtual void preparePipeline();
public:
    struct Pipelines {
        VkPipeline skybox;
        VkPipeline pbr;
        VkPipeline pbrAlphaBlend;
    } pipelines;

    VkDescriptorSet         dsScene;

    struct Textures {
        vke::TextureCubeMap environmentCube;
        vke::Texture2D      lutBrdf;
        vke::TextureCubeMap irradianceCube;
        vke::TextureCubeMap prefilteredCube;
    } textures;

    vkglTF::Model               skybox;
    std::vector<vkglTF::Model>  models;

    pbrRenderer ();
    virtual ~pbrRenderer();

    virtual void destroy();

    void renderPrimitive(vkglTF::Primitive &primitive, VkCommandBuffer commandBuffer);

    void prepareModels();

    virtual void draw(VkCommandBuffer cmdBuff);


};

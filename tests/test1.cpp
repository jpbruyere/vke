#include "vke.hpp"
#include "camera.hpp"
#include "phyInfos.hpp"
#include "pbrrenderer2.hpp"
#include "VulkanSwapChain.hpp"

class Test : public vke::engine_t
{
public:
    pbrRenderer*   sceneRenderer = nullptr;
    vkglTF::Model* mod;

    Test() : engine_t(1024, 768, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        title = "Vulkan Chess glTf 2.0 PBR";
        camera->type = vke::Camera::CameraType::firstperson;
        camera->movementSpeed = 8.0f;
        camera->rotationSpeed = 0.25f;
        camera->setPerspective(45.0f, (float)width / (float)height, 0.1f, 256.0f);
        camera->setRotation({ .0f, 0.0f, 2.5f });
        camera->setPosition({ .0f, .0f, 0.f });


        settings.validation = true;

        phyInfos->enabledFeatures.samplerAnisotropy
                = phyInfos->features.samplerAnisotropy;
        phyInfos->selectQueue (phyInfos->pQueue);
    }

    ~Test()
    {
        delete(sceneRenderer);
    }

    virtual void prepare() {
        sceneRenderer = new pbrRenderer();

        sceneRenderer->create(device, renderTarget, sharedUBOs);

        sceneRenderer->models.resize(1);
        mod = &sceneRenderer->models[0];

        //mod->loadFromFile ("data/models/chess.gltf", device, true);
        mod->loadFromFile ("data/models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf", device, true);

        //mod->addInstance("frame", glm::translate(glm::mat4(1.0),       glm::vec3( 0,0,0)));
        mod->addOneInstanceOfEach();

        sceneRenderer->prepareModels();
        sceneRenderer->buildCommandBuffer();
    }

    void render () {
        if (!prepared)
            return;

        prepareFrame();

        sceneRenderer->submit(device->queue, &swapChain->presentCompleteSemaphore, 1);
        VK_CHECK_RESULT(swapChain->queuePresent(device->queue, sceneRenderer->drawComplete));

        //update();
    }
};

Test *test;


int main(const int argc, const char *argv[])
{

    for (size_t i = 0; i < argc; i++) { Test::args.push_back(argv[i]); };
    test = new Test();
    test->start();
    delete(test);
    return 0;
}

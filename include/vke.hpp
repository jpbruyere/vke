/*
* Vulkan Example base class
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <chrono>
#include <sys/stat.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <exception>
#include <assert.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <array>
#include <vector>
#include <numeric>
#include <algorithm>

#include "vulkan/vulkan.h"

#include "GLFW/glfw3.h"

namespace vke {
    class  engine_t;
    class  swap_chain_t;
    struct device_t;
    struct Texture;
    struct render_target_t;
    struct buffer_t;
    class ShadingContext;
    struct Resource;
    class Camera;
    struct vkPhyInfo;
}

#if defined(__linux__)
#define KEY_ESCAPE 0x9
#define KEY_F1 0x43
#define KEY_F2 0x44
#define KEY_F3 0x45
#define KEY_F4 0x46
#define KEY_W 0x19
#define KEY_A 0x26
#define KEY_S 0x27
#define KEY_D 0x28
#define KEY_P 0x21
#define KEY_SPACE 0x41
#define KEY_KPADD 0x56
#define KEY_KPSUB 0x52
#define KEY_B 0x38
#define KEY_F 0x29
#define KEY_L 0x2E
#define KEY_N 0x39
#define KEY_O 0x20
#define KEY_T 0x1C
#endif

// AngelCode .fnt format structs and classes
struct bmchar {
    uint32_t x, y;
    uint32_t width;
    uint32_t height;
    int32_t xoffset;
    int32_t yoffset;
    int32_t xadvance;
    uint32_t page;
};
std::array<bmchar, 255> parsebmFont(const std::string& fileName);

namespace vke {

    class engine_t
    {
    private:
        float       fpsTimer = 0.0f;
        uint32_t    frameCounter = 0;

        bool        viewUpdated = false;

        std::string getWindowTitle();

#if DEBUG
        PFN_vkCreateDebugReportCallbackEXT  vkCreateDebugReportCallback;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback;
        VkDebugReportCallbackEXT            debugReportCallback;
#endif

        void _createInstance (const std::string& app_name, std::vector<const char*>& extentions);
    protected:
        vkPhyInfo*          phyInfos;
        GLFWwindow*         window;

        swap_chain_t*       swapChain;
        render_target_t*    renderTarget;
        std::string         title = "Vulkan Example";
        std::string         name = "vulkanExample";

        virtual void windowResize();
    public:
        static std::vector<const char*> args;

        VkInstance      instance;
        device_t*       device;
        VkSurfaceKHR    surface;


        uint32_t    lastFPS     = 0;
        float       frameTimer  = 1.0f;
        bool        prepared    = false;
        uint32_t    width, height;
        Camera*     camera;
        glm::vec2   mousePos;
        bool        paused      = false;

        struct Settings {
            bool validation = false;
            bool fullscreen = false;
            bool vsync = false;
            bool multiSampling = true;
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_4_BIT;
        } settings;

        struct UniformBuffers {
            buffer_t* matrices;
            buffer_t* params;
        } sharedUBOs;

        struct MVPMatrices {
            glm::mat4 projection;
            glm::mat4 view3;
            glm::mat4 view;
            glm::vec3 camPos;
        } mvpMatrices;

        struct LightingParams {
            glm::vec4 lightDir = glm::vec4(0.0f, -0.5f, -0.5f, 1.0f);
            float exposure = 4.5f;
            float gamma = 1.0f;
            float prefilteredCubeMipLevels;
        } lightingParams;

        glm::vec3 rotation = glm::vec3(0.0f, 135.0f, 0.0f);

        struct GamePadState {
            glm::vec2 axisLeft = glm::vec2(0.0f);
            glm::vec2 axisRight = glm::vec2(0.0f);
        } gamePadState;

        bool mouseButtons[3] = {false,false,false};


        engine_t(uint32_t width, uint32_t height,
                 VkPhysicalDeviceType preferedGPU = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
        virtual ~engine_t();

        virtual void start();

        virtual void render() = 0;
        virtual void viewChanged();
        virtual void keyDown(uint32_t key);
        virtual void keyUp(uint32_t key);
        virtual void keyPressed(uint32_t key);
        virtual void handleMouseMove(int32_t x, int32_t y);
        virtual void handleMouseButtonDown(int buttonIndex);
        virtual void handleMouseButtonUp(int buttonIndex);
        virtual void prepare();

        void prepareUniformBuffers();
        void updateUniformBuffers();
        void updateParams();

        void prepareFrame();
    };
}

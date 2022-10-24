#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include "function/render/VulkanSetup.h"

namespace VlkEngine {
    class VulkanEngine {
    public:
        void Run(); 

        VulkanEngine(int w, int h, std::string name);
        ~VulkanEngine();

    private:        
        void InitEngine(); 
        void StartEngine(); 
        void MainLoop();
        void ShutDownEngine();
        
        // window
        GLFWwindow* window;
        std::string windowName;
        const uint32_t width;
        const uint32_t height;

        // render
        VulkanSetup* vulkanSetup;
    };
}

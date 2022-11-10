#pragma once

#include "function/render/VulkanSetup.h"
#include "function/render/RenderPipline.h"
#include "function/render/RenderBuffer.h"
#include "function/render/VulkanSyncObject.h"
#include "function/render/RenderDescriptor.h"

namespace VlkEngine {
    

    class VulkanEngine {
    public:
        void Run(); 
        
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

        VulkanEngine(int w, int h, std::string name);
        ~VulkanEngine();

    private:        
        void InitEngine(); 
        void StartEngine(); 
        void MainLoop();
        void ShutDownEngine();

        void DrawFrame();

        void RecordCommandBuffer(uint32_t imageIndex);
        void WindowSurfaceChange();
        void UpdateUniformBuffer(uint32_t currentImage);
        
        // window
        GLFWwindow* window;
        std::string windowName;
        const uint32_t width;
        const uint32_t height;

        // render
        uint32_t currentFrame = 0;
        VulkanSetup* vulkanSetup;
        RenderPipline* renderPipline;
        RenderBuffer* renderBuffer;
        RenderDescriptor* renderDescriptor;
        VulkanSyncObject* vulkanSyncObject;
        bool framebufferResized = false;
    };
}

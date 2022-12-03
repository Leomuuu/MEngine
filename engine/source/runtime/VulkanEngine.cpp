#include "VulkanEngine.h"
namespace VlkEngine {
    VulkanEngine::VulkanEngine(int w, int h, std::string name):width(w),height(h),windowName(name)
    {
        InitEngine();
    }

    VulkanEngine::~VulkanEngine()
    {
        delete vulkanBase;
    }

    void VulkanEngine::Run()
    {
        StartEngine();
        MainLoop();
        ShutDownEngine();
    }

	void VulkanEngine::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
        auto app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
	}

	void VulkanEngine::MouseMovecallback(GLFWwindow* window, double xpos, double ypos)
	{
        auto app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
        InputSystem* inputSystem = app->inputSystem;

        if (inputSystem!=nullptr) {
            inputSystem->MouseMovement(window, xpos, ypos);
        }
	}

	void VulkanEngine::KeyInputcallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
	}

	void VulkanEngine::InitEngine()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback); 
        glfwSetKeyCallback(window, KeyInputcallback); 
        glfwSetCursorPosCallback(window, MouseMovecallback);

        camera = new Camera(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::radians(45.0f),0.1f,10.0f);

        inputSystem = new InputSystem(camera);

        modelManager = new ModelManager();
        modelManager->SetModel(
            "C:/Users/MU/Desktop/Graduation Project/code/MEngine/engine/asset/model/lpshead/head.OBJ",
            "C:/Users/MU/Desktop/Graduation Project/code/MEngine/engine/asset/model/lpshead/lambertian.jpg");

        vulkanBase = new VulkanBase(window, this);
        
    }

    void VulkanEngine::StartEngine()
    {
        vulkanBase->StartVulkan();
    }

    void VulkanEngine::MainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            DrawFrame();
            inputSystem->ProcessInput(window);
        }
        vkDeviceWaitIdle(vulkanBase->device);
    }

    void VulkanEngine::ShutDownEngine()
    {
        vulkanBase->ShutDownVulkan();
        
        glfwDestroyWindow(window);
        glfwTerminate();
    }

	
	void VulkanEngine::DrawFrame()
	{
        vkWaitForFences(vulkanBase->device, 1, &(vulkanBase->inFlightFences[currentFrame]), VK_TRUE, UINT64_MAX);
        
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(vulkanBase->device, vulkanBase->swapChain,
            UINT64_MAX, vulkanBase->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            WindowSurfaceChange();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        UpdateUniformBuffer(currentFrame);
        
        vkResetFences(vulkanBase->device, 1, &(vulkanBase->inFlightFences[currentFrame]));
        
        vkResetCommandBuffer(vulkanBase->commandBuffers[currentFrame], 0);
        RecordCommandBuffer(imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { vulkanBase->imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(vulkanBase->commandBuffers[currentFrame]);
        VkSemaphore signalSemaphores[] = { vulkanBase->renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if (vkQueueSubmit(vulkanBase->graphicsQueue, 1, &submitInfo, vulkanBase->inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { vulkanBase->swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(vulkanBase->presentQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            WindowSurfaceChange();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

	void VulkanEngine::RecordCommandBuffer(uint32_t imageIndex)
	{
        VkCommandBuffer commandBuffer = vulkanBase->commandBuffers[currentFrame];

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vulkanBase->renderPass;
        renderPassInfo.framebuffer = vulkanBase->swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = vulkanBase->swapChainExtent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanBase->graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(vulkanBase->swapChainExtent.width);
        viewport.height = static_cast<float>(vulkanBase->swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = vulkanBase->swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { vulkanBase->vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, vulkanBase->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanBase->pipelineLayout,
            0, 1, &((vulkanBase->descriptorSets)[currentFrame]), 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(modelManager->indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
	}

	void VulkanEngine::WindowSurfaceChange()
	{
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(vulkanBase->device);

        vulkanBase->DestroyDepthResource();
        vulkanBase->DestroyFramebuffers();
        vulkanBase->DestroyImageViews();
        vulkanBase->DestroySwapChain();

        vulkanBase->CreateSwapChain();
        vulkanBase->CreateImageViews();
        vulkanBase->CreateDepthResource();
        vulkanBase->CreateFramebuffers();
	}

	void VulkanEngine::UpdateUniformBuffer(uint32_t currentImage)
	{
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        MVPMatrix ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = camera->GetViewMatrix();
        ubo.proj = glm::perspective(camera->Fov, vulkanBase->swapChainExtent.width / (float)(vulkanBase->swapChainExtent.height), camera->zNear, camera->zFar);
        ubo.proj[1][1] *= -1;

        FragUniform fragubo{};
        fragubo.viewPosition = camera->camPosition;
        fragubo.lightPosition =lightPosition;

        memcpy(vulkanBase->uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
        memcpy(vulkanBase->fraguniformBuffersMapped[currentImage], &fragubo, sizeof(fragubo));


    }
}

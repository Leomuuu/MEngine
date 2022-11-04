#pragma once

#include "VulkanSetup.h"
#include "RenderPipline.h"

namespace VlkEngine {
	class RenderBuffer {
		friend class VulkanEngine;
		public:
			RenderBuffer(VulkanSetup* vulkansetup,RenderPipline* renderpipline);
		private:
			VulkanSetup* vulkanSetup;
			RenderPipline* renderPipline;

		public:
			void CreateBuffers();
			void DestroyBuffers();

			
		private:
			// frame buffer
			void CreateFramebuffers();
			void DestroyFramebuffers();
			// command pool
			void CreateCommandPool();
			void DestroyCommandPool();
			// command buffer
			void CreateCommandBuffer();
			void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		private:
			// frame buffer
			std::vector<VkFramebuffer> swapChainFramebuffers;
			// command pool
			VkCommandPool commandPool;
			// command buffer
			std::vector<VkCommandBuffer> commandBuffers;

	};

}
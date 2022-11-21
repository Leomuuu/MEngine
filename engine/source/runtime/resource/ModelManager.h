#include "../function/platform/FileService.h"
#include <tiny_obj_loader.h>
#include <unordered_map>
#include "../function/render/RenderHeader.h"

namespace VlkEngine {
	class ModelManager {
		friend class VulkanEngine;
	private:
		std::string modelPath;
		std::string texturePath;

	public:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;


	public:
		void SetModel(std::string modelpath, std::string texturepath);
		stbi_uc* LoadModelTexture(int* texWidth, int* texHeight, int* texChannels);

	private:
		void LoadModel();
	};

}
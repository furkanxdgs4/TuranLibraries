#pragma once
#include "Vulkan_Includes.h"
#include "GFX/GFX_Core.h"

#include "Renderer/Vulkan_Resource.h"
#include "Renderer/VK_GPUContentManager.h"
#include "Renderer/Vulkan_Renderer_Core.h"


namespace Vulkan {
	class VK_API Vulkan_Core : public GFX_API::GFX_Core {
	public:
		Vulkan_States VK_States;

		//Initialization Processes

		void Check_Computer_Specs(vector<GFX_API::GPUDescription>& GPUdescs);
		void Save_Monitors(vector<GFX_API::MonitorDescription>& Monitors);

		virtual void Check_Errors() override;
		virtual bool GetTextureTypeLimits(const GFX_API::Texture_Properties& Properties, GFX_API::TEXTUREUSAGEFLAG UsageFlag, unsigned int GPUIndex,
			unsigned int& MAXWIDTH, unsigned int& MAXHEIGHT, unsigned int& MAXDEPTH, unsigned int& MAXMIPLEVEL) override;
		virtual void GetSupportedAllocations_ofTexture(const GFX_API::Texture_Description& TEXTURE_desc, unsigned int GPUIndex, 
			unsigned int& SupportedMemoryTypesBitset) override;
		//Window Operations

		virtual GFX_API::GFXHandle CreateWindow(const GFX_API::WindowDescription& Desc, GFX_API::GFXHandle* SwapchainTextureHandles, GFX_API::Texture_Properties& SwapchainTextureProperties) override;
		virtual void Change_Window_Resolution(GFX_API::GFXHandle Window, unsigned int width, unsigned int height) override;
		vector<GFX_API::GFXHandle>& Get_WindowHandles();

		//Input (Keyboard-Controller) Operations
		virtual void Take_Inputs() override;

		//Callbacks
		static void GFX_Error_Callback(int error_code, const char* description);
		static void Window_ResizeCallback(GLFWwindow* window, int WIDTH, int HEIGHT);

		void Create_Instance();
		void Setup_Debugging();


		//Destroy Operations
		virtual void Destroy_GFX_Resources() override;

		Vulkan_Core(vector<GFX_API::MonitorDescription>& Monitors, vector<GFX_API::GPUDescription>& GPUs, TuranAPI::Threading::JobSystem* JobSystem);
		//All of the sizes should be in bytes
		TAPIResult Start_SecondStage(unsigned char GPUIndex, const vector<GFX_API::MemoryType>& Allocations);
		virtual ~Vulkan_Core();
	};

}
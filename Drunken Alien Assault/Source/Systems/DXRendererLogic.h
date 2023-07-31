#pragma once
// The rendering system is responsible for drawing all objects
#ifndef VULKANRENDERERLOGIC_H
#define VULKANRENDERERLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include <d3dcompiler.h>
#include <wrl/client.h>
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"

class DXRendererLogic
{
	// shared connection to the main ECS engine
	std::shared_ptr<flecs::world> game;
	// non-ownership handle to configuration settings
	std::weak_ptr<const GameConfig> gameConfig;
	// handle to our running ECS systems
	flecs::system startDraw;
	flecs::system updateDraw;
	flecs::system completeDraw;
	// Used to query screen dimensions
	GW::SYSTEM::GWindow window;
	// Vulkan resources used for rendering
	GW::GRAPHICS::GDirectX11Surface DXSurface;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexData = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> vertexFormat = nullptr;
	//VkDescriptorPool descriptorPool = nullptr;
	//VkPipeline pipeline = nullptr;     PIPELINE should come from the currentModel 
	//VkPipelineLayout pipelineLayout = nullptr;
	//std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> uniformHandle;
	//std::vector<VkDeviceMemory> uniformData;  //this is a buffer I didnt use last month, how will we use it?
	//std::vector<VkDescriptorSet> descriptorSet;
	// used to trigger clean up of vulkan resources
	GW::CORE::GEventReceiver shutdown;
public:
	// attach the required logic to the ECS 
	bool Init(std::shared_ptr<flecs::world> _game,
		std::weak_ptr<const GameConfig> _gameConfig,
		GW::GRAPHICS::GDirectX11Surface _DXSurface,
		GW::SYSTEM::GWindow _window);
	// control if the system is actively running
	bool Activate(bool runSystem);
	// release any resources allocated by the system
	bool Shutdown();
private:
	// Loading funcs
	bool LoadShaders();
	//bool LoadUniforms();
	bool LoadGeometry();
	bool SetupPipeline();
	bool SetupDrawcalls();
	// Unloading funcs
	bool FreeVulkanResources();
	// Utility funcs
	std::string ShaderAsString(const char* shaderFilePath);
private:
	// Uniform Data Definitions
	static constexpr unsigned int Instance_Max = 240;
	struct INSTANCE_UNIFORMS
	{
		GW::MATH::GMATRIXF instance_transforms[Instance_Max];
		GW::MATH::GVECTORF instance_colors[Instance_Max];
	}instanceData;
	// how many instances will be drawn this frame
	int draw_counter = 0;
};

#endif



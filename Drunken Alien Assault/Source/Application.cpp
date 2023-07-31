#include "Application.h"
#define Textures "../../Blender Levels/Textures";

// open some Gateware namespaces for conveinence 
// NEVER do this in a header file!
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;

Renderer Application::myRenderer;

bool Application::Init()
{
	eventPusher.Create();

	// load all game settigns
	gameConfig = std::make_shared<GameConfig>();
	// create the ECS system
	game = std::make_shared<flecs::world>();
	// init all other systems
	if (InitWindow() == false)
		return false;
	if (InitInput() == false)
		return false;
	if (InitAudio() == false)
		return false;
	if (InitGraphics() == false)
		return false;
	if (InitLevel() == false)
		return false;
	if (InitEntities() == false)
		return false;
	if (InitSystems() == false)
		return false;
	return true;
}

bool Application::Run(int level)
{
	float clr[] = { 0, 150 / 255.0f, 150 / 255.0f, 1 };
	myRenderer = Renderer(level, window, DXSurface, game, log, levelObjs, immediateInput);
	D3D11_VIEWPORT screen{ 0, 0, 800, 600, 0, 1 };
	auto& e = levelObjs.allObjectsInLevel;
	while (+window.ProcessWindowEvents())
	{
		
		IDXGISwapChain* swap;
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		ID3D11DepthStencilView* depth;
		if (+DXSurface.GetImmediateContext((void**)&con) &&
			+DXSurface.GetRenderTargetView((void**)&view) &&
			+DXSurface.GetDepthStencilView((void**)&depth) &&
			+DXSurface.GetSwapchain((void**)&swap))
		{
			con->ClearRenderTargetView(view, clr);
			con->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 0);
			if (GameLoop() == false)
			{
				playerDead = true;
				return true;
			}
			if (playerSystem.Update(audioMuted, &playerDead, &lostALife))
			{
				//myRenderer.DebugCamera(immediateInput, gamePads);
				/*if (&lostALife)
				{
					for (auto& e : levelObjs.allObjectsInLevel) {
						if (e.GetName() == "craft_speederD")
						{
							auto playerRef = game->lookup("Player");
							e.MD.worldMatrix = playerRef.get_ref<Transform>().get()->transform;
							e.DrawModel(DXSurface, window, screen, e.MD.worldMatrix);
							break;
						}
					}
				}*/

				myRenderer.ScrollingCam(playerSystem.GetCameraSpeed(), &lostALife);
				myRenderer.RenderMain();

				swap->Present(1, 0);
				// release incremented COM reference counts
				swap->Release();
				view->Release();
				depth->Release();
				con->Release();
			}
			else //This block only happens if the player is dead
			{
				swap->Present(1, 0);
				// release incremented COM reference counts
				swap->Release();
				view->Release();
				depth->Release();
				con->Release();
				return true;
			}
			
			
		}
	}
	return false;
}

bool Application::Shutdown()
{
	// disconnect systems from global ECS
	if (playerSystem.Shutdown() == false)
		return false;
	if (levelSystem.Shutdown() == false)
		return false;
	if (DXRenderingSystem.Shutdown() == false)
		return false;
	if (physicsSystem.Shutdown() == false)
		return false;
	if (bulletSystem.Shutdown() == false)
		return false;
	if (enemySystem.Shutdown() == false)
		return false;

	return true;
}

bool Application::InitWindow()
{
	// grab settings
	int width = gameConfig->at("Window").at("width").as<int>();
	int height = gameConfig->at("Window").at("height").as<int>();
	int xstart = gameConfig->at("Window").at("xstart").as<int>();
	int ystart = gameConfig->at("Window").at("ystart").as<int>();
	std::string title = gameConfig->at("Window").at("title").as<std::string>();
	// open window
	if (+window.Create(xstart, ystart, width, height, GWindowStyle::WINDOWEDLOCKED) &&
		+window.SetWindowName(title.c_str())) {
		return true;
	}
	return false;
}

bool Application::InitInput()
{
	if (-gamePads.Create())
		return false;
	if (-immediateInput.Create(window))
		return false;
	if (-bufferedInput.Create(window))
		return false;
	return true;
}

bool Application::InitAudio()
{
	if (-audioEngine.Create())
		return false;
	return true;

	
}

bool Application::InitGraphics()
{
	GReturn g;
	window.SetWindowName("Drunken Alien Assault");
	msgs.Create([&](const GW::GEvent& e)
		{
			GW::SYSTEM::GWindow::Events q;
			e.Read(q);
		});
	window.Register(msgs);
	if (+DXSurface.Create(window, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
	{
		return true;
	}

	return 0;
}

bool Application::InitLevel()
{
	log.Create("logfile.txt");
	log.EnableConsoleLogging(true);

	return true;
}

bool Application::InitEntities()
{
	// Load bullet prefabs
	if (weapons.Load(game, gameConfig, audioEngine) == false)
		return false;
	// Load the enemy entities
	//if (enemies.Load(game, gameConfig, audioEngine) == false)
	//	return false;

	return true;
}

bool Application::InitSystems()
{
	// connect systems to global ECS
	if (playerSystem.Init(game, gameConfig, immediateInput, bufferedInput,
		gamePads, audioEngine, eventPusher) == false)
		return false;
	if (levelSystem.Init(game, gameConfig, audioEngine) == false)
		return false;
	if (DXRenderingSystem.Init(game, gameConfig, DXSurface, window) == false)
		return false;
	if (physicsSystem.Init(game, gameConfig) == false)
		return false;
	if (bulletSystem.Init(game, gameConfig) == false)
		return false;
	if (enemySystem.Init(game, gameConfig, eventPusher) == false)
		return false;

	return true;
}

bool Application::GameLoop()
{
	// compute delta time and pass to the ECS system
	static auto start = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(
		std::chrono::steady_clock::now() - start).count();
	start = std::chrono::steady_clock::now();
	// let the ECS system run
	return game->progress(static_cast<float>(elapsed));
}


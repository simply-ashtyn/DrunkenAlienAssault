#ifndef APPLICATION_H
#define APPLICATION_H
#pragma once
#define level1txt "../Final Level 1/GameLevel1.txt"
#define level2txt "../Final Level 2/GameLevel2.txt"
#define level3txt "../Final Level 3/GameLevel3.txt"
#define level1M "../Final Level 1/Level1Models"
#define level2M "../Final Level 2/Level2Models"
#define level3M "../Final Level 3/Level3Models"

// include events
#include "Events/Playevents.h"
// Contains our global game settings
#include "GameConfig.h"
// Load all entities+prefabs used by the game 
#include "Entities/BulletData.h"
#include "Entities/EnemyData.h"
// Include all systems used by the game and their associated components
#include "Systems/PlayerLogic.h"
#include "Systems/DXRendererLogic.h"
#include "Systems/LevelLogic.h"
#include "Systems/PhysicsLogic.h"
#include "Systems/BulletLogic.h"
#include "Systems/EnemyLogic.h"
//#include"load_object_oriented.h"
#include "renderer.h"


// Allocates and runs all sub-systems essential to operating the game
class Application 
{
	// gateware libs used to access operating system
	GW::SYSTEM::GWindow window; // gateware multi-platform window
	GW::GRAPHICS::GDirectX11Surface DXSurface;
	
	
	// GLog
	GW::SYSTEM::GLog log;

	//GW::GRAPHICS::GVulkanSurface vulkan; // gateware vulkan API wrapper
	GW::INPUT::GController gamePads; // controller support
	GW::INPUT::GInput immediateInput; // twitch keybaord/mouse
	GW::INPUT::GBufferedInput bufferedInput; // event keyboard/mouse

	// third-party gameplay & utility libraries
	std::shared_ptr<flecs::world> game; // ECS database for gameplay

	// ECS Entities and Prefabs that need to be loaded
	BulletData weapons;
	EnemyData enemies;
	// specific ECS systems used to run the game
	DXRendererLogic DXRenderingSystem;
	LevelLogic levelSystem;
	PhysicsLogic physicsSystem;
	BulletLogic bulletSystem;
	EnemyLogic enemySystem;
	// EventGenerator for Game Events
	GW::CORE::GEventGenerator eventPusher;
	GW::CORE::GEventResponder msgs;

	static Renderer myRenderer; 

public:
	bool rendered = false;
	// level Objects
	Level_Objects levelObjs;
	bool lostALife = false;
	int selectLevel = 0;
	bool playerDead = false;
	bool swappingLevels = false;
	bool audioMuted;
	PlayerLogic playerSystem;
	GW::AUDIO::GAudio audioEngine; // can create music & sound effects
	std::shared_ptr<GameConfig> gameConfig; // .ini file game settings
	bool Init();
	bool Run(int level);
	bool Shutdown();
	
private:
	bool InitWindow();
	bool InitInput();
	bool InitAudio();
	bool InitLevel();
	bool InitGraphics();
	bool InitEntities();
	bool InitSystems();
	bool GameLoop();
};

#endif 
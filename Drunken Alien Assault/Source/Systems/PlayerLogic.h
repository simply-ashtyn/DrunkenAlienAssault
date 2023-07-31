#pragma once
// The player system is responsible for allowing control over the main ship(s)
#ifndef PLAYERLOGIC_H
#define PLAYERLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Components/Physics.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Gameplay.h"
#include "../Entities/Prefabs.h"
#include "../Events/Playevents.h"

class PlayerLogic
{
	// shared connection to the main ECS engine
	std::shared_ptr<flecs::world> game;
	// non-ownership handle to configuration settings
	std::weak_ptr<const GameConfig> gameConfig;
	// handle to our running ECS system
	flecs::system playerSystem;
	flecs::entity player;
	float playerSpeed;
	float cameraSpeed;
	///limit number of times a cheat can be used
	int hyperSpeed;
	int immortality;
	// permananent handles input systems
	GW::INPUT::GInput immediateInput;
	GW::INPUT::GBufferedInput bufferedInput;
	GW::INPUT::GController controllerInput;
	// permananent handle to audio system
	GW::AUDIO::GAudio audioEngine;
	GW::AUDIO::GMusic currentTrack;
	GW::AUDIO::GSound gameOver;
	GW::AUDIO::GSound crash;
	GW::AUDIO::GSound livesRemainig1;
	GW::AUDIO::GSound livesRemainig2;
	// key press event cache (saves input events)
	// we choose cache over responder here for better ECS compatibility
	GW::CORE::GEventCache pressEvents;
	GW::INPUT::GBufferedInput::Events k_event;
	GW::INPUT::GBufferedInput::EVENT_DATA k_data;
	GW::INPUT::GController::Events c_event;
	GW::INPUT::GController::EVENT_DATA c_data;
	// varibables used for charged shot timing
	float chargeStart = 0, chargeEnd = 0, chargeTime;
	// event responder
	GW::CORE::GEventResponder onExplode;

	bool collision;
	float camSpeed;

public:
	float playerHealth = 10;
	int playerLives = 3;
	bool paused;
	bool music;
	// attach the required logic to the ECS 
	bool Init(std::shared_ptr<flecs::world> _game,
		std::weak_ptr<const GameConfig> _gameConfig,
		GW::INPUT::GInput _immediateInput,
		GW::INPUT::GBufferedInput _bufferedInput,
		GW::INPUT::GController _controllerInput,
		GW::AUDIO::GAudio _audioEngine,
		GW::CORE::GEventGenerator _eventPusher);
	// control if the system is actively running
	bool Activate(bool runSystem);
	bool Update(bool musicMuted, bool* playerDead, bool* lostALife);
	// release any resources allocated by the system
	bool Shutdown();
	float GetCameraSpeed();
	void pauseMusic();
	void resumeMusic();
private:
	// how big the input cache can be each frame
	static constexpr unsigned int Max_Frame_Events = 32;
	// helper routines
	void MovePlayer(float time);
	bool FireLasers(flecs::world& stage, GW::MATH::GVECTORF origin);
	void TakeDamage(bool* playerDead, bool* lostALife);
	void Reset(bool* playerDead, bool* lostALife);
	void doNothing();
};

#endif
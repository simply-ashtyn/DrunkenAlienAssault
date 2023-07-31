#include "PlayerLogic.h"

using namespace GW::INPUT; // input libs
using namespace GW::AUDIO; // audio libs

// Connects logic to traverse any players and allow a controller to manipulate them
bool PlayerLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::INPUT::GInput _immediateInput,
	GW::INPUT::GBufferedInput _bufferedInput,
	GW::INPUT::GController _controllerInput,
	GW::AUDIO::GAudio _audioEngine,
	GW::CORE::GEventGenerator _eventPusher)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	immediateInput = _immediateInput;
	bufferedInput = _bufferedInput;
	controllerInput = _controllerInput;
	audioEngine = _audioEngine;

	// Create an event cache for when the spacebar/'A' button is pressed
	pressEvents.Create(Max_Frame_Events); // even 32 is probably overkill for one frame
	// register for keyboard and controller events
	bufferedInput.Register(pressEvents);
	controllerInput.Register(pressEvents);
	///Audio
	currentTrack.Create("../Music/background1.wav", audioEngine, 0.15f);
	currentTrack.Play();
	crash.Create("../SoundFX/collision.wav", audioEngine, 0.15f);
	gameOver.Create("../SoundFX/GameOver.wav", audioEngine, 0.3f);
	livesRemainig1.Create("../SoundFX/lives1.wav", audioEngine, 0.3f);
	livesRemainig2.Create("../SoundFX/lives2.wav", audioEngine, 0.3f);

	///Gameplay
	player = game->lookup("Player");
	paused = true;
	collision = false;

	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	int width = (*readCfg).at("Window").at("width").as<int>();
	playerSpeed = (*readCfg).at("Player1").at("speed").as<float>();
	chargeTime = (*readCfg).at("Player1").at("chargeTime").as<float>();
	cameraSpeed = (*readCfg).at("Camera").at("speed").as<float>();

	hyperSpeed = 2;
	immortality = 1;

	// create the on explode handler
	onExplode.Create([this](const GW::GEvent& e) {
		PLAY_EVENT event; PLAY_EVENT_DATA eventData;
		if (+e.Read(event, eventData)) {
			// only in here if event matches
			std::cout << "Enemy Was Destroyed!\n";
		}
		});
	_eventPusher.Register(onExplode);

	return true;
}

bool PlayerLogic::Update(bool musicMuted, bool* playerDead, bool* lostALife) //add bool for collision
{
	if (paused)
	{
		if (cameraSpeed > 0)
		{
			camSpeed = cameraSpeed;
		}
		cameraSpeed = 0;
		if (!musicMuted)
		{
			currentTrack.Pause();
		}
	}
	if (!paused)
	{
		if (cameraSpeed == 0)
		{
			cameraSpeed = camSpeed;
		}
		if (!musicMuted)
		{
			currentTrack.Resume();
		}
	}
	if (!collision)
	{
		static auto start = std::chrono::high_resolution_clock::now();
		float elapsedTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();
		MovePlayer(elapsedTime);
		TakeDamage(playerDead, lostALife);
		start = std::chrono::high_resolution_clock::now();
	}
	if (*playerDead)
	{
		return false;
	}
	return true;
}

// Free any resources used to run this system
bool PlayerLogic::Shutdown()
{
	playerSystem.destruct();
	game.reset();
	gameConfig.reset();

	return true;
}

// Toggle if a system's Logic is actively running
bool PlayerLogic::Activate(bool runSystem)
{
	if (playerSystem.is_alive()) {
		(runSystem) ?
			playerSystem.enable()
			: playerSystem.disable();
		return true;
	}
	return false;
}

void PlayerLogic::MovePlayer(float time)
{
	if (player = game->lookup("Player"))
	{
		auto& playerTransform = player.get_ref<Transform>();

		if (!paused)
		{
			float perFrameSpeed = playerSpeed * time;
			float yChange = 0;
			float zChange = 0;

			float wState = 0, upState = 0;
			float sState = 0, downState = 0;
			float aState = 0, leftState = 0;
			float dState = 0, rightState = 0;

			float leftStickStateY = 0;
			///asdw
			immediateInput.GetState(G_KEY_W, wState);
			immediateInput.GetState(G_KEY_S, sState);
			immediateInput.GetState(G_KEY_A, aState);
			immediateInput.GetState(G_KEY_D, dState);
			if (wState > 0 || sState > 0 || aState > 0 || dState > 0)
			{
				yChange = (wState - sState + leftStickStateY) * 5;
				zChange = (dState - aState) * 5;
			}
			///arrows
			immediateInput.GetState(G_KEY_UP, upState);
			immediateInput.GetState(G_KEY_DOWN, downState);
			immediateInput.GetState(G_KEY_LEFT, leftState);
			immediateInput.GetState(G_KEY_RIGHT, rightState);
			if (upState > 0 || downState > 0 || leftState > 0 || rightState > 0)
			{
				yChange = (upState - downState + leftStickStateY) * 5;
				zChange = (rightState - leftState) * 5;
			}

			controllerInput.GetState(0, G_LY_AXIS, leftStickStateY);

			if (wState > 0 || sState > 0)
			{
				std::cout << "yChange: " << yChange << std::endl;
				std::cout << "yTrans: " << playerTransform->transform.row4.y << std::endl;
			}

			playerTransform->transform.row4.y += yChange * perFrameSpeed;
			playerTransform->transform.row4.z += zChange * perFrameSpeed;

			///Experiment for keeping player in camera view - doesn't work
			//playerTransform->transform.row4.y = G_LARGER(playerTransform->transform.row4.y + yChange * perFrameSpeed, -0.8F);
			//playerTransform->transform.row4.y = G_SMALLER(playerTransform->transform.row4.y + yChange * perFrameSpeed, 0.8F);
			//playerTransform->transform.row4.z = G_LARGER(playerTransform->transform.row4.z + zChange * perFrameSpeed, -0.8F);
			//playerTransform->transform.row4.z = G_SMALLER(playerTransform->transform.row4.z + zChange * perFrameSpeed, 0.8F);
		}


		// pull any waiting events from the event cache and process them
		GW::GEvent event;
		while (+pressEvents.Pop(event)) {
			bool fire = false;
			GController::Events controller;
			GBufferedInput::Events keyboard;
			// these will only happen when needed
			if (+event.Read(keyboard, k_data)) {
				if (keyboard == GBufferedInput::Events::KEYPRESSED) {
					if (k_data.data == G_KEY_P)
					{
						paused = !paused;
						std::cout << "pause" << std::endl;
					}
					if (k_data.data == G_KEY_SPACE && !paused) {
						fire = true;
						FireLasers(*game, playerTransform->transform.row4);
						chargeStart = game->time();
					}
					if (k_data.data == G_KEY_I && !paused) //Invincibility Cheat
					{
						if (immortality > 0)
						{
							player.add<Immortal>();
							std::cout << "IMMORTAL" << std::endl;
							playerHealth = 99999;
							immortality--;
						}
					}
					if (k_data.data == G_KEY_H && !paused) //Hyper Speed
					{
						if (hyperSpeed > 0)
						{
							std::cout << "HYPER SPEED ENGAGED" << std::endl;
							playerSpeed *= 2;
							cameraSpeed *= 7;
							hyperSpeed--;
						}
					}
				}
				if (keyboard == GBufferedInput::Events::KEYRELEASED) {
					if (k_data.data == G_KEY_SPACE && !paused) {
						chargeEnd = game->time();
						if (chargeEnd - chargeStart >= chargeTime) {
							fire = true;
						}
					}
				}
			}
			else if (+event.Read(controller, c_data)) {
				if (controller == GController::Events::CONTROLLERBUTTONVALUECHANGED) {
					if (c_data.inputValue > 0 && c_data.inputCode == G_START_BTN && !paused)
					{
						paused = !paused;
					}
					if (c_data.inputValue > 0 && c_data.inputCode == G_RIGHT_TRIGGER_AXIS && !paused)
					{
						fire = true;
						FireLasers(*game, playerTransform->transform.row4);
						controllerInput.StartVibration(0, 0.f, 0.2f, 0.2f);
					}
					if (c_data.inputValue > 0 && c_data.inputCode == G_LEFT_SHOULDER_BTN && !paused)//Invincibility Cheat
					{
						if (immortality > 0)
						{
							playerHealth = 99999;
							immortality--;
						}
					}
					if (c_data.inputValue > 0 && c_data.inputCode == G_NORTH_BTN && !paused)//Hyper Speed
					{
						if (hyperSpeed > 0)
						{
							playerSpeed *= 5;
							cameraSpeed *= 7;
							hyperSpeed--;
						}
					}
				}
			}
		}
	}
}

// play sound and launch two laser rounds
bool PlayerLogic::FireLasers(flecs::world& stage, GW::MATH::GVECTORF origin)
{
	std::cout << "PLAYER firing laser" << std::endl;

	// Grab the prefab for a laser round
	flecs::entity bullet;
	RetreivePrefab("Lazer Bullet", bullet);
	origin.z += 0.05f;
	origin.y -= 0.05f;
	auto laserLeft = stage.entity().is_a(bullet)
		.set<GW::MATH::GVECTORF>(origin);
	origin.y += 0.1f;
	auto laserRight = stage.entity().is_a(bullet)
		.set<GW::MATH::GVECTORF>(origin);
	// if this shot is charged
	if (chargeEnd - chargeStart >= chargeTime) {
		chargeEnd = chargeStart;
		laserLeft.set<ChargedShot>({ 2 })
			.set<Material>({ 1,0,0 });
		laserRight.set<ChargedShot>({ 2 })
			.set<Material>({ 1,0,0 });
	}

	// play the sound of the Lazer prefab
	GW::AUDIO::GSound shoot = *bullet.get<GW::AUDIO::GSound>();
	shoot.Play();

	return true;
}

void PlayerLogic::TakeDamage(bool* playerDead, bool* lostALife)
{
	//collision check
	//std::cout << "Table type: " << player.table().type().str() << std::endl;
	if (player.has<Crashed>() && immortality == 1)
	{
		player.disable();
		cameraSpeed = 0;
		currentTrack.Stop();
		crash.Play();
		controllerInput.StartVibration(0, 0.f, 1.f, 1.f);
		playerLives -= 1;
		*lostALife = true;
		Reset(playerDead, lostALife);
	}
	
	if(player.has<Immortal>())
	{
		auto& enemy = game->lookup("Enemy");
		if (enemy.has<Collidable>())
		{
			auto& eTrasform = game->get_ref<Transform>();
			eTrasform->transform.row4.y =10000;
			eTrasform->transform.row4.z = 10000;
			enemy.add<Dead>();
		}
	}
}

float PlayerLogic::GetCameraSpeed()
{
	return cameraSpeed;
}

void PlayerLogic::Reset(bool* playerDead, bool* lostALife)
{
	if (playerLives > 0)
	{
		player.remove<Crashed>();
		player.enable();
		if (playerLives == 1)
		{
			livesRemainig1.Play();
			//Reset positions
			/*if (player = game->lookup("Player"))
			{
				auto& playerTransform = player.get_ref<Transform>();
				std::cout << playerTransform->transform.data + '\n';
				playerTransform->transform.row4.y = -18.809;
				playerTransform->transform.row4.z = 1.4703;
				std::cout << playerTransform->transform.data + '\n';
				doNothing();
			}*/
			//camera = {7.3589, -19.295, 1.5778}
		}
		else if (playerLives == 2)
		{
			livesRemainig2.Play();
			//reset positions
			//if (player = game->lookup("Player"))
			//{
			//	auto& playerTransform = player.get_ref<Transform>();
			//	std::cout << playerTransform->transform.row4.y;
			//	std::cout << playerTransform->transform.row4.z + '\n';
			//	playerTransform->transform.row4.y = 1.4703;
			//	playerTransform->transform.row4.z = 0;
			//	std::cout << playerTransform->transform.row4.y;
			//	std::cout << playerTransform->transform.row4.z + '\n';
			//	doNothing();
			//	//-18.809
			//}
		}
	}
	else
	{
		gameOver.Play();
		doNothing();
		*playerDead = true;
		*lostALife = false;
	}
}

void PlayerLogic::doNothing() ///can be removed  with proper shut down code
{
	for (int i = 0; i < 20000; i++)
	{
		//stalling
		std::cout << "i" << '\n';
	}
}

void PlayerLogic::pauseMusic()
{
	currentTrack.isPlaying(music);
	if (music)
	{
		currentTrack.Pause();
	}
}

void PlayerLogic::resumeMusic()
{
	currentTrack.isPlaying(music);
	if (!music)
	{
		currentTrack.Play();
	}
}
#include "EnemyLogic.h"

// Connects logic to traverse any players and allow a controller to manipulate them
bool EnemyLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::CORE::GEventGenerator _eventPusher)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	eventPusher = _eventPusher;

	// destroy any bullets that have the CollidedWith relationship
	game->system<Enemy, Health>("Enemy System")
		.each([this](flecs::entity e, Enemy, Health& h) {
		flecs::entity player = game->lookup("Player");
		if (PlayerInRange(player, e))
		{
			///shoot!
		}
		// if you have no health left be destroyed
		if (e.get<Health>()->value <= 0) {
			// play explode sound
			e.destruct();
			PLAY_EVENT_DATA x;
			GW::GEvent explode;
			explode.Write(PLAY_EVENT::ENEMY_DESTROYED, x);
			eventPusher.Push(explode);
		}

		if (player.has<Immortal>() && e.has<Collidable>())
		{
			e.add<Dead>();
			e.get_ref<Transform>()->transform.row4.y = 100000;
			e.get_ref<Transform>()->transform.row4.z = 100000;

		}
			});

	return true;
}

bool EnemyLogic::PlayerInRange(flecs::entity _player, flecs::entity _enemy)
{
	///Check distance between enemy and player, activate enemy
	float distance = _enemy.get_ref<Transform>()->transform.row4.z - _player.get_ref<Transform>()->transform.row4.z;
	if (distance < 10) //arbitrary number, need to experiment
	{
		///player in range - ACTIVATE
		return true;
	}
	else
	{
		return false;
	}
}

bool EnemyLogic::FireLasers(flecs::world& stage, GW::MATH::GVECTORF origin)
{
	std::cout << "ENEMY firing laser" << std::endl;

	// Grab the prefab for a laser round
	flecs::entity bullet;
	RetreivePrefab("Lazer Bullet", bullet);
	origin.z -= 0.05f;
	origin.y -= 0.05f;
	auto laserLeft = stage.entity().is_a(bullet)
		.set<GW::MATH::GVECTORF>(origin);
	origin.y += 0.1f;
	auto laserRight = stage.entity().is_a(bullet)
		.set<GW::MATH::GVECTORF>(origin);

	return true;
}

// Free any resources used to run this system
bool EnemyLogic::Shutdown()
{
	game->entity("Enemy System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool EnemyLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Enemy System").enable();
	}
	else {
		game->entity("Enemy System").disable();
	}
	return false;
}

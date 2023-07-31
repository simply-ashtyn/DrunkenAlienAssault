#pragma once
// The level system is responsible for transitioning the various levels in the game
#ifndef PHYSICSLOGIC_H
#define PHYSICSLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Components/Physics.h"

class PhysicsLogic
{
	// shared connection to the main ECS engine
	std::shared_ptr<flecs::world> game;
	// non-ownership handle to configuration settings
	std::weak_ptr<const GameConfig> gameConfig;
	// used to cache collision queries (2D)
	//flecs::query<Collidable, Position, Orientation> queryCache;
	// used to cache collision queries (3D)
	flecs::query<Collidable, Transform> queryCache3D;

	//filter info?
	flecs::filter<Collidable, Transform> filterCache3D;

	// defines what to be tested
	static constexpr unsigned polysize = 4;
	struct SHAPE {
		GW::MATH2D::GVECTOR2F poly[polysize];
		flecs::entity owner;
	};
	struct SPHERE {
		GW::MATH::GSPHEREF sphere;
		flecs::entity owner;
	};
	// vector used to save/cache all active collidables
	std::vector<SHAPE> testCache;
	// vector used to save/cache all active collidables in 3D space
	std::vector<SPHERE> testCache3D;
public:
	// attach the required logic to the ECS 
	bool Init(std::shared_ptr<flecs::world> _game,
		std::weak_ptr<const GameConfig> _gameConfig);
	// control if the system is actively running
	bool Activate(bool runSystem);
	// release any resources allocated by the system
	bool Shutdown();
};

#endif

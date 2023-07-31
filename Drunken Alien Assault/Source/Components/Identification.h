// define all ECS components related to identification
#ifndef IDENTIFICATION_H
#define IDENTIFICATION_H

	// TAGS TO MARK ENTITES THAT WE WILL NEED
	struct Models {};
	struct Player {};
	struct Bullet {};
	struct Enemy {};
	struct Boss {};
	struct Lives {};
	struct Collide{}; //for everything the player can crash into

	// COMPONENTS, WHAT DOES AN ENTITY HAVE **will need to convert to obj oriented**
	struct Boundry { GW::MATH::GVECTORF collisionBox[8]; };

#endif
// define all ECS components related to movement & collision
#ifndef PHYSICS_H
#define PHYSICS_H

	// component types should be *strongly* typed for proper queries
	// typedef is tempting but it does not help templates/functions resolve type
struct Position { GW::MATH2D::GVECTOR2F value; };
struct Velocity { GW::MATH2D::GVECTOR2F value; };
struct Orientation { GW::MATH2D::GMATRIX2F value; };
struct Acceleration { GW::MATH2D::GVECTOR2F value; };
struct ModelName { std::string name; };
struct Transform { GW::MATH::GMATRIXF transform; };

// Individual TAGs
struct Collidable {};
struct Crashed {};
struct Immortal {};
struct Dead {};

// ECS Relationship tags
struct CollidedWith {};

#endif
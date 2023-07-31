#include "PhysicsLogic.h"

bool PhysicsLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	// **** MOVEMENT ****
	// update velocity by acceleration
	game->system<Velocity, const Acceleration>("Acceleration System")
		.each([](flecs::entity e, Velocity& v, const Acceleration& a) {
		GW::MATH2D::GVECTOR2F accel;
		GW::MATH2D::GVector2D::Scale2F(a.value, e.delta_time(), accel);
		GW::MATH2D::GVector2D::Add2F(accel, v.value, v.value);
			});
	// update position by velocity
	game->system<Position, const Velocity>("Translation System")
		.each([](flecs::entity e, Position& p, const Velocity& v) {
		GW::MATH2D::GVECTOR2F speed;
		GW::MATH2D::GVector2D::Scale2F(v.value, e.delta_time(), speed);
		// adding is simple but doesn't account for orientation
		GW::MATH2D::GVector2D::Add2F(speed, p.value, p.value);
			});
	// **** CLEANUP ****
	// clean up any objects that end up offscreen
	game->system<const Position>("Cleanup System")
		.each([](flecs::entity e, const Position& p) {
		if (p.value.x > 1.5f || p.value.x < -1.5f ||
			p.value.y > 1.5f || p.value.y < -1.5f) {
			e.destruct();
		}
			});
	// **** COLLISIONS ****
	// due to wanting to loop through all collidables at once, we do this in two steps:
	// 1. A System will gather all collidables into a shared std::vector
	// 2. A second system will run after, testing/resolving all collidables against each other
	//queryCache = game->query<Collidable, Position, Orientation>(); // - Original Norri implementation in 2D
	//queryCache3D = game->query<Collidable, Transform>(); //3D Implementation
	filterCache3D = game->filter<Collidable, Transform>();
	// only happens once per frame at the very start of the frame
	struct CollisionSystem {}; // local definition so we control iteration count (singular)
	game->entity("Detect-Collisions").add<CollisionSystem>();
	game->system<CollisionSystem>()
		.each([this](CollisionSystem& s) {

		//// This the base shape all objects use & draw, this might normally be a component collider.(ex:sphere/box)
		//constexpr GW::MATH2D::GVECTOR2F poly[polysize] = {
		//	{ -0.5f, -0.5f }, { 0, 0.5f }, { 0.5f, -0.5f }, { 0, -0.25f }
		//};

		//Updated base shape information
		//Currently, a sphere that is, by default, centered at (0,0,0), and with a radius of 2 units
		constexpr GW::MATH::GSPHEREF sphereCollider = {
			{0.0f, 0.0f, 0.0f, 0.60f}
		};
		//// collect any and all collidable objects
		//queryCache.each([this, poly](flecs::entity e, Collidable& c, Position& p, Orientation& o) {
		//	// create a 3x3 matrix for transformation
		//	GW::MATH2D::GMATRIX3F matrix = {
		//		o.value.row1.x, o.value.row1.y, 0,
		//		o.value.row2.x, o.value.row2.y, 0,
		//		p.value.x, p.value.y, 1
		//	};
		//	SHAPE polygon; // compute buffer for this objects polygon
		//	// This is critical, if you want to store an entity handle it must be mutable
		//	polygon.owner = e; // allows later changes
		//	for (int i = 0; i < polysize; ++i) {
		//		GW::MATH2D::GVECTOR3F v = { poly[i].x, poly[i].y, 1 };
		//		GW::MATH2D::GMatrix2D::MatrixXVector3F(matrix, v, v);
		//		polygon.poly[i].x = v.x;
		//		polygon.poly[i].y = v.y;
		//	}
		//	// add to vector
		//	testCache.push_back(polygon);
		//});
		//TODO
		//collect any and all collidable objects - 3D
		// Iterate over all of the collidable objects
		filterCache3D.each([this, sphereCollider](flecs::entity e, Collidable& c, Transform& t)
			{
				//Create a SPHERE object
				SPHERE sphere;
				//Assign the owner
				sphere.owner = e;
				//std::cout << e.get_ref<ModelName>()->name << std::endl;
				//Transform the sphere to the position of the entity
				sphere.sphere.x = e.get_ref<Transform>()->transform.row4.x;
				sphere.sphere.y = e.get_ref<Transform>()->transform.row4.y;
				sphere.sphere.z = e.get_ref<Transform>()->transform.row4.z;
				sphere.sphere.radius = 0.5f;
				//Add to testCache3D vector
				testCache3D.push_back(sphere);
			});
		//
		//// loop through the testCache resolving all collisions
		//for (int i = 0; i < testCache.size(); ++i) {
		//	// the inner loop starts at the entity after you so you don't double check collisions
		//	for (int j = i + 1; j < testCache.size(); ++j) {

		//		// test the two world space polygons for collision
		//		// possibly make this cheaper by leaving one of them local and using an inverse matrix
		//		GW::MATH2D::GCollision2D::GCollisionCheck2D result;
		//		GW::MATH2D::GCollision2D::TestPolygonToPolygon2F(
		//			testCache[i].poly, polysize, testCache[j].poly, polysize, result);
		//		if (result == GW::MATH2D::GCollision2D::GCollisionCheck2D::COLLISION) {
		//			// Create an ECS relationship between the colliders
		//			// Each system can decide how to respond to this info independently
		//			testCache[j].owner.add<CollidedWith>(testCache[i].owner);
		//			testCache[i].owner.add<CollidedWith>(testCache[j].owner);
		//		}
		//	}
		//}
		//// wipe the test cache for the next frame (keeps capacity intact)
		//testCache.clear();

		// loop through the testCache3D resolving all collisions
		for (int i = 0; i < testCache3D.size(); ++i)
		{
			//the inner loop starts at the entity after you so you don't double check collisions by mistake
			for (int j = i + 1; j < testCache3D.size(); ++j)
			{
				//test the two world space spheres for collision
				GW::MATH::GCollision::GCollisionCheck result;
				GW::MATH::GCollision::TestSphereToSphereF(
					testCache3D[i].sphere, testCache3D[j].sphere, result);
				if (result == GW::MATH::GCollision::GCollisionCheck::COLLISION) {
					if (testCache3D[i].owner.name() == "Player")
					{
						if (testCache3D[i].owner.has<Immortal>())
						{
							testCache3D[j].owner.get_ref<Transform>()->transform.row4.y = 10000;
							testCache3D[j].owner.get_ref<Transform>()->transform.row4.z = 10000;

						}
						//std::cout << "Crash Detected between " << testCache3D[i].owner.name() << " and " << testCache3D[j].owner.name() << std::endl;
						//collision = true;
					}					
					//Create an ECS relationship between the colliders
					//Each system can decide how to respond to this info independtly
					testCache3D[j].owner.add<CollidedWith>(testCache3D[i].owner); ///tag not getting assigned to actual owner??
					testCache3D[j].owner.add<Crashed>(); ///tag not getting assigned to actual owner??
					testCache3D[i].owner.add<CollidedWith>(testCache3D[j].owner);
				}
			}
		}
		// wipe the test cache for the next frame (keep capacity intact)
		testCache3D.clear();
			});
	return true;
}

bool PhysicsLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Acceleration System").enable();
		game->entity("Translation System").enable();
		game->entity("Cleanup System").enable();
	}
	else {
		game->entity("Acceleration System").disable();
		game->entity("Translation System").disable();
		game->entity("Cleanup System").disable();
	}
	return true;
}

bool PhysicsLogic::Shutdown()
{
	queryCache3D.destruct(); // fixes crash on shutdown
	game->entity("Acceleration System").destruct();
	game->entity("Translation System").destruct();
	game->entity("Cleanup System").destruct();
	return true;
}

#ifndef PLAYEVENTS_H
#define PLAYEVENTS_H

enum PLAY_EVENT {
	ENEMY_DESTROYED,
	EVENT_COUNT
};
struct PLAY_EVENT_DATA {
	flecs::id entity_id; // which entity was affected?
};

#endif
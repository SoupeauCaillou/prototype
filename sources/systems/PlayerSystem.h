#pragma once

#include "systems/System.h"

struct PlayerComponent {
	PlayerComponent():score(0){};

	int score;
	Color playerColor;
};

#define thePlayerSystem PlayerSystem::GetInstance()
#define PLAYER(e) thePlayerSystem.Get(e)

UPDATABLE_SYSTEM(Player)
};

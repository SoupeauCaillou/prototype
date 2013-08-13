#pragma once

#include "systems/System.h"

class NetworkAPI;

struct PlayerComponent {
	PlayerComponent():id(-1),score(0){};

    int id;
	int score;
	Color playerColor;
};

#define thePlayerSystem PlayerSystem::GetInstance()
#define PLAYER(e) thePlayerSystem.Get(e)

UPDATABLE_SYSTEM(Player)
    public:
        Entity GetMyself(bool networkMode, const NetworkAPI* networkAPI);
};

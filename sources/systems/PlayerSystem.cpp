#include "PlayerSystem.h"

#include "api/NetworkAPI.h"

INSTANCE_IMPL(PlayerSystem);

PlayerSystem::PlayerSystem() : ComponentSystemImpl <PlayerComponent>("Player") {
	PlayerComponent pc;
	componentSerializer.add(new Property<int>("id", OFFSET(id, pc)));
    componentSerializer.add(new Property<int>("score", OFFSET(score, pc)));
}

void PlayerSystem::DoUpdate(float) {
	// FOR_EACH_ENTITY_COMPONENT(Plane, e, pc)
	// }
}


Entity PlayerSystem::GetMyself(bool networkMode, const NetworkAPI* networkAPI) {
    // Retrieve all players
    std::vector<Entity> players = thePlayerSystem.RetrieveAllEntityWithComponent();
    Entity myPlayer = 0;

    // Pick mine
    bool gameMaster = (!networkMode || networkAPI->amIGameMaster());

    for_each(players.begin(), players.end(), [&myPlayer, gameMaster] (Entity e) -> void {
        if (PLAYER(e)->id == (gameMaster ? 0 : 1)) myPlayer = e;
    });
    if (myPlayer) {
        // LOGI_EVERY_N(100, "Found my player. Entity: " << myPlayer);
    } else {
        LOGW("Cannot find my player :'( (" << players.size());
    }
    return myPlayer;
}

#if SAC_INGAME_EDITORS

#include "ParatroopersDebugConsole.h"

#include "base/Log.h"
#include "base/ObjectSerializer.h"

#include "util/ScoreStorageProxy.h"

#include <glm/gtc/random.hpp>

#include "ParatroopersGame.h"
#include "api/StorageAPI.h"
ParatroopersGame* ParatroopersDebugConsole::_game = 0;

void ParatroopersDebugConsole::callbackSubmitRandomScore(void* arg) {
    int count = *(int*)arg;
    while (--count > -1) {
        ScoreStorageProxy ssp;
        ssp.setValue("name", "random123", true);
        ssp.setValue("time", ObjectSerializer<float>::object2string(glm::linearRand(0.0f, 100.0f)), false);
        _game->gameThreadContext->storageAPI->saveEntries(&ssp);
    }
}

void ParatroopersDebugConsole::init(ParatroopersGame* game) {
    _game = game;

    TwEnumVal numberToGenerate[] = { {1, "1"}, {10, "10"}, {100, "100"} };
    REGISTER_ONE_ARG(SubmitRandomScore, numberToGenerate)
}
#endif

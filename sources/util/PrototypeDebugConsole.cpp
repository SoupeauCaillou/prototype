#if SAC_INGAME_EDITORS

#include "PrototypeDebugConsole.h"

#include "base/Log.h"
#include "base/ObjectSerializer.h"

#include "util/ScoreStorageProxy.h"

#include <glm/gtc/random.hpp>

#include "PrototypeGame.h"
#include "api/StorageAPI.h"
PrototypeGame* PrototypeDebugConsole::_game = 0;

void PrototypeDebugConsole::callbackSubmitRandomScore(void* arg) {
    LOGI("clicked!");

    int count = *(int*)arg;
    while (--count > -1) {
        ScoreStorageProxy ssp;
        ssp.setValue("name", "random123", true);
        ssp.setValue("time", ObjectSerializer<float>::object2string(glm::linearRand(0.0f, 100.0f)), false);
        _game->gameThreadContext->storageAPI->saveEntries(&ssp);
    }
}

void PrototypeDebugConsole::init(PrototypeGame* game) {
    _game = game;

    TwEnumVal numberToGenerate[] = { {1, "1"}, {10, "10"}, {100, "100"} };
    REGISTER(SubmitRandomScore, numberToGenerate)
}
#endif

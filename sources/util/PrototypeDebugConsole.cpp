#if SAC_INGAME_EDITORS

#include "PrototypeDebugConsole.h"
#include "util/DebugConsole.h"

#include "base/Log.h"

int PrototypeDebugConsole::test = 0;

static void submitRandomScore(void* arg) {
    LOGI("clicked!");

    if (arg) {
        LOGI("arg: " << *((int*)arg));
    }
}

void PrototypeDebugConsole::init() {
        TwEnumVal submitRandomScoreModes[] = {
            {1, "1"},
            {10, "10"}
        };
        TwType type = TwDefineEnum("Argument", submitRandomScoreModes, 2);

        DebugConsole::Instance().registerMethod("submitRandomScore", &submitRandomScore, type, &PrototypeDebugConsole::test);
}
#endif

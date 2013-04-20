#if SAC_INGAME_EDITORS

#include "PrototypeDebugConsole.h"
#include "util/DebugConsole.h"

#include "base/Log.h"

static void submitRandomScore(void* args) {
    LOGI("clicked!");
    /*std::list<std::string> list = (std::list<std::string>)args;
    for (std::string arg : args) {
        LOGI(arg);
    }*/
}

void PrototypeDebugConsole::init() {
        DebugConsole::Instance().registerMethod("submitRandomScore", &submitRandomScore);
        DebugConsole::Instance().registerMethod("random1", &submitRandomScore);
        DebugConsole::Instance().registerMethod("random29191", &submitRandomScore);
}
#endif

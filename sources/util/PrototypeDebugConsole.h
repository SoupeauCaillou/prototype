#if SAC_INGAME_EDITORS

#include "util/DebugConsole.h"

class PrototypeGame;

class PrototypeDebugConsole {
    public:
        static void init(PrototypeGame* game);
        static void callbackSubmitRandomScore(void* arg);

    private:
        //to interact with the game
        static PrototypeGame* _game;
};

#endif

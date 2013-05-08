#if SAC_INGAME_EDITORS

#include "util/DebugConsole.h"

class ParatroopersGame;

class ParatroopersDebugConsole {
    public:
        static void init(ParatroopersGame* game);
        static void callbackSubmitRandomScore(void* arg);

    private:
        //to interact with the game
        static ParatroopersGame* _game;
};

#endif

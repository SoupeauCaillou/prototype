#pragma once
#include "base/Game.h"
#include "util/HexSpatialGrid.h"
#include "base/StateMachine.h"
#include "base/StateMachine.inl"
#include "scenes/Scenes.h"

namespace Case {
    enum Enum {
        Empty  = 1 << 0,
        Rock   = 1 << 1,
        Dog     = 1 << 2,
        Sheep  = 1 << 3,
        Start   = 1 << 4,
        End     = 1 << 5,
    };
}

static const GridPos invalidGridPos(-100, -100);

class SacHelloWorldGame : public Game {
    public:
        SacHelloWorldGame();
        void init(const uint8_t* in = 0, int size = 0);
        bool wantsAPI(ContextAPI::Enum api) const;

        void updateMovesCount(int value);
    private:
        void tick(float dt);
    public:
        StateMachine<Scene::Enum> sceneStateMachine;
        HexSpatialGrid* grid;

        const char* level;
        int movesCount; /*number of moves done by user for current level*/
        Entity movesCountE; /*display movesCount to user*/
};

#pragma once
#include "base/Game.h"
#include "util/HexSpatialGrid.h"

class SacHelloWorldGame : public Game {
    public:
        SacHelloWorldGame();
        void init(const uint8_t* in = 0, int size = 0);
        bool wantsAPI(ContextAPI::Enum api) const;

    public:
        typedef enum {
            Empty  = 1 << 0,
            Rock   = 1 << 1,
            Dog     = 1 << 2,
            Sheep  = 1 << 3,
            Start   = 1 << 4,
            End     = 1 << 5,
        } GameElement;

    private:
        void tick(float dt);

    private:
        HexSpatialGrid grid;
};

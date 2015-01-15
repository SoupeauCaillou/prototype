#pragma once
#include "base/Game.h"
#include "util/HexSpatialGrid.h"

class SacHelloWorldGame : public Game {
    public:
        SacHelloWorldGame();
        void init(const uint8_t* in = 0, int size = 0);

    private:
        void tick(float dt);

    private:
        HexSpatialGrid grid;
};

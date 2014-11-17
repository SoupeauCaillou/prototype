#pragma once
#include "base/Game.h"

#include "scenes/Scenes.h"
#include "base/StateMachine.h"

class MyTestGame : public Game {
public:
        void init(const uint8_t* in = 0, int size = 0);

    private:
        void tick(float dt);

        StateMachine<Scene::Enum>* sceneStateMachine;

    public:
        Entity playerUnit;
};

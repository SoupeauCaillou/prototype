/*
    This file is part of Bzzz.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Bzzz is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Bzzz is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bzzz.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>
#include <vector>
#include "base/StateMachine.h"
#include "states/Scenes.h"

#include "base/Color.h"
#include "base/Game.h"
#include "base/GameContext.h"

#include "util/DataFileParser.h"
#include "util/FaderHelper.h"

class BzzzGame : public Game {
    public:
        BzzzGame(int argc, char** argv);

        bool wantsAPI(ContextAPI::Enum api) const;
        void sacInit(int windowW, int windowH);
        void init(const uint8_t* in = 0, int size = 0);
        void tick(float dt);
        void togglePause(bool activate);
        bool willConsumeBackEvent();
        void backPressed();
        void quickInit() {}

        // make some bees when clicking on a button
        void beesPopping(Entity fromBtn);

        Entity camera;
        FaderHelper faderHelper;

        Color playerColors[5];
        int score[4];
        int playerActive[4];
        Entity playerButtons[4];
        std::vector<Entity> bees;
        std::vector<Entity> selected;

        DataFileParser parameters;

    private:
        StateMachine<Scene::Enum> sceneStateMachine;
};

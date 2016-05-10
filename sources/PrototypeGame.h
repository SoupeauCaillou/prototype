/*
    This file is part of Prototype.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Prototype is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Prototype is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Prototype.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>
#include <vector>
#include "base/StateMachine.h"
#include "gen/Scenes.h"

#include "base/Color.h"
#include "base/Game.h"
#include "base/GameContext.h"

#include "util/DataFileParser.h"
#include "util/FaderHelper.h"

#include <sac/tweak.h>

class PrototypeGame : public Game {
    public:
    PrototypeGame();

    void init(const uint8_t* in = 0, int size = 0);
    void tick(float dt);

    FaderHelper faderHelper;

    private:
    StateMachine<Scene::Enum> sceneStateMachine;

    public:
    Entity guy[4];
    Entity battleground;
};

extern void deletePlayer(Entity e);
extern void deletePlayerAndWeapons(Entity e);

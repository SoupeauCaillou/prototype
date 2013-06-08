/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <string>
#include <vector>
#include "base/StateMachine.h"
#include "states/Scenes.h"

#include "util/SpatialGrid.h"

#include "base/Game.h"
#include "base/GameContext.h"

#include "systems/RenderingSystem.h"

#include "api/LocalizeAPI.h"
#include "api/AdAPI.h"

#include "VisibilityManager.h"

class PrototypeGame : public Game {
	public:
		PrototypeGame(int argc, char** argv);

        bool wantsAPI(ContextAPI::Enum api) const;
        void sacInit(int windowW, int windowH);
        void init(const uint8_t* in = 0, int size = 0);
        void quickInit();
        void tick(float dt);
		void togglePause(bool activate);
        bool willConsumeBackEvent();
		void backPressed();

	// Game datas
	public:
        Entity camera;
        std::string serverIp, nickname;
    	SpatialGrid grid;
    	Entity activeCharacter;
        VisibilityManager visibilityManager;

        // Scene variables
        std::list<Entity> walls;
        std::list<Entity> yEnnemies;
        std::list<Entity> bEnnemies;
        std::list<Entity> players;
        std::list<Entity> objs;
        Entity background;
        Entity banner, turn, points;
        Entity humanPlayer, aiPlayer;

    private:
        StateMachine<Scene::Enum> sceneStateMachine;
};

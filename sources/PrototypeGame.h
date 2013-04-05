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

#include "base/Game.h"
#include "base/GameContext.h"

#include "systems/RenderingSystem.h"

#include "api/LocalizeAPI.h"
#include "api/AdAPI.h"
#include "api/NameInputAPI.h"

#include "states/StateManager.h"

class PrototypeGame : public Game {
	public:
		PrototypeGame();

        bool wantsAPI(ContextAPI::Enum api) const;
        void sacInit(int windowW, int windowH);
        void init(const uint8_t* in = 0, int size = 0);
        void quickInit();
        void tick(float dt);
		void togglePause(bool activate);
        bool willConsumeBackEvent();
		void backPressed();
        void changeState(State::Enum newState);

        Entity camera;
	private:
        State::Enum currentState, overrideNextState;
        std::map<State::Enum, StateManager*> state2manager;
        TransitionStateManager transitionManager;
};

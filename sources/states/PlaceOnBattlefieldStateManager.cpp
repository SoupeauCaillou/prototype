/*
    This file is part of Brikwars.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer

    RecursiveRunner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    RecursiveRunner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "StateManager.h"

#include "base/EntityManager.h"
#include "base/Vector2.h"
#include "systems/RenderingSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/FighterSystem.h"
#include "systems/PickableSystem.h"
#include <vector>
#include <sstream>

struct PlaceOnBattlefieldStateManager::PlaceOnBattlefieldStateManagerDatas {

};

PlaceOnBattlefieldStateManager::PlaceOnBattlefieldStateManager(PrototypeGame* game) : StateManager(State::PlaceOnBattlefield, game) {
    datas = new PlaceOnBattlefieldStateManagerDatas;
}

PlaceOnBattlefieldStateManager::~PlaceOnBattlefieldStateManager() {
    delete datas;
}

void PlaceOnBattlefieldStateManager::setup() {

}


///----------------------------------------------------------------------------//
///--------------------- ENTER SECTION ----------------------------------------//
///----------------------------------------------------------------------------//
void PlaceOnBattlefieldStateManager::willEnter(State::Enum from) {

}

bool PlaceOnBattlefieldStateManager::transitionCanEnter(State::Enum) {
    return true;
}


void PlaceOnBattlefieldStateManager::enter(State::Enum) {
    std::vector<Entity> fighters = theFighterSystem.RetrieveAllEntityWithComponent();
}

///----------------------------------------------------------------------------//
///--------------------- UPDATE SECTION ---------------------------------------//
///----------------------------------------------------------------------------//
void PlaceOnBattlefieldStateManager::backgroundUpdate(float) {
}

State::Enum PlaceOnBattlefieldStateManager::update(float dt) {
    return State::PlaceOnBattlefield; //BattleColorPick;
}


///----------------------------------------------------------------------------//
///--------------------- EXIT SECTION -----------------------------------------//
///----------------------------------------------------------------------------//
void PlaceOnBattlefieldStateManager::willExit(State::Enum) {
}

bool PlaceOnBattlefieldStateManager::transitionCanExit(State::Enum) {
    return true;
}

void PlaceOnBattlefieldStateManager::exit(State::Enum) {
    
}

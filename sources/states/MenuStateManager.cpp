/*
	This file is part of RecursiveRunner.

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

#include <sstream>
#include <vector>

struct MenuStateManager::MenuStateManagerDatas {

};

MenuStateManager::MenuStateManager(PrototypeGame* game) : StateManager(State::Menu, game) {
    datas = new MenuStateManagerDatas;
}

MenuStateManager::~MenuStateManager() {
    delete datas;
}

void MenuStateManager::setup() {

}


///----------------------------------------------------------------------------//
///--------------------- ENTER SECTION ----------------------------------------//
///----------------------------------------------------------------------------//
void MenuStateManager::willEnter(State::Enum from) {

}

bool MenuStateManager::transitionCanEnter(State::Enum) {
    return true;
}


void MenuStateManager::enter(State::Enum) {

}


///----------------------------------------------------------------------------//
///--------------------- UPDATE SECTION ---------------------------------------//
///----------------------------------------------------------------------------//
void MenuStateManager::backgroundUpdate(float) {
}

State::Enum MenuStateManager::update(float) {
    return State::Equipment;
}


///----------------------------------------------------------------------------//
///--------------------- EXIT SECTION -----------------------------------------//
///----------------------------------------------------------------------------//
void MenuStateManager::willExit(State::Enum) {

}

bool MenuStateManager::transitionCanExit(State::Enum) {
    return true;
}

void MenuStateManager::exit(State::Enum) {
    
}

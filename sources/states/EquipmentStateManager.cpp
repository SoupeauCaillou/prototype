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

#include "base/Vector2.h"
#include "systems/RenderingSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/FighterSystem.h"
#include "systems/EquipmentSystem.h"
#include <vector>

struct EquipmentStateManager::EquipmentStateManagerDatas {
    
};

EquipmentStateManager::EquipmentStateManager(PrototypeGame* game) : StateManager(State::Equipment, game) {
    datas = new EquipmentStateManagerDatas;
}

EquipmentStateManager::~EquipmentStateManager() {
    delete datas;
}

void EquipmentStateManager::setup() {

}


///----------------------------------------------------------------------------//
///--------------------- ENTER SECTION ----------------------------------------//
///----------------------------------------------------------------------------//
void EquipmentStateManager::willEnter(State::Enum from) {

}

bool EquipmentStateManager::transitionCanEnter(State::Enum) {
    return true;
}


void EquipmentStateManager::enter(State::Enum) {
    // The screen is divided in 8 columns and 6 rows
    //   - 2 first columns x 6 rows = fighters of P1
    //   - 2 last col x 6 rows = fighters of P2
    //   - in between : equipment available
    const Vector2 size(theRenderingSystem.screenW / 8, -theRenderingSystem.screenH / 6);
    const Vector2 offset = Vector2(-theRenderingSystem.screenW * 0.5, theRenderingSystem.screenH * 0.5) + size * 0.5;
    std::vector<Entity> fighters = theFighterSystem.RetrieveAllEntityWithComponent();
    std::vector<Entity> players = thePlayerSystem.RetrieveAllEntityWithComponent();

    // Fighters
    for (unsigned i=0; i<players.size(); i++) {
        int count = 0;
        for (unsigned j=0; j<fighters.size(); j++) {
            if (FIGHTER(fighters[j])->player == players[i]) {
                int col = count / 6;
                int row = count - col * 6;
                if (i == 1) col = 7 - col;
                TRANSFORM(fighters[j])->position = offset + Vector2(col * size.X, row * size.Y);
                count++;
            }
        }
    }

    // Equipment
    std::vector<Entity> equipments = theEquipmentSystem.RetrieveAllEntityWithComponent();
    const Vector2 minV = offset + Vector2(2 * size.X, 5 * size.Y);
    const Vector2 maxV = offset + Vector2(5 * size.X, 0);
    for (unsigned i=0; i<equipments.size(); i++) {
        TRANSFORM(equipments[i])->position = MathUtil::RandomVector(minV, maxV);
        TRANSFORM(equipments[i])->rotation = MathUtil::RandomFloat(MathUtil::TwoPi);
    }
}


///----------------------------------------------------------------------------//
///--------------------- UPDATE SECTION ---------------------------------------//
///----------------------------------------------------------------------------//
void EquipmentStateManager::backgroundUpdate(float) {
}

State::Enum EquipmentStateManager::update(float) {
    return State::Equipment;
}


///----------------------------------------------------------------------------//
///--------------------- EXIT SECTION -----------------------------------------//
///----------------------------------------------------------------------------//
void EquipmentStateManager::willExit(State::Enum) {

}

bool EquipmentStateManager::transitionCanExit(State::Enum) {
    return true;
}

void EquipmentStateManager::exit(State::Enum) {
    
}

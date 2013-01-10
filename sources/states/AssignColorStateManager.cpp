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

struct AssignColorStateManager::AssignColorStateManagerDatas {
    int activePlayer;
    std::vector<Entity> players, fighters;
    Entity colors[4];
    int pickedColorIndex;
};

AssignColorStateManager::AssignColorStateManager(PrototypeGame* game) : StateManager(State::AssignColor, game) {
    datas = new AssignColorStateManagerDatas;
}

AssignColorStateManager::~AssignColorStateManager() {
    delete datas;
}

void AssignColorStateManager::setup() {
    const Vector2 size(theRenderingSystem.screenW / 8, -theRenderingSystem.screenH / 6);

    for (int i=0; i<4; i++) {
        Entity color = datas->colors[i] = theEntityManager.CreateEntity();
        ADD_COMPONENT(color, Transformation);
        TRANSFORM(color)->size = size * 1.5;
        TRANSFORM(color)->z = 0.5;
        Vector2 p(i/2, i - 2 * (i/2));
        TRANSFORM(color)->position = -size + size * p * 2; 
        ADD_COMPONENT(color, Rendering);
        std::stringstream s;
        s << "group_" << i;
        RENDERING(color)->color = Color(s.str());
        ADD_COMPONENT(color, Pickable);
    }
    datas->pickedColorIndex = -1;
}


///----------------------------------------------------------------------------//
///--------------------- ENTER SECTION ----------------------------------------//
///----------------------------------------------------------------------------//
void AssignColorStateManager::willEnter(State::Enum from) {

}

bool AssignColorStateManager::transitionCanEnter(State::Enum) {
    return true;
}


void AssignColorStateManager::enter(State::Enum) {
    for (int i=0; i<4; i++) {
        RENDERING(datas->colors[i])->hide = false;
        PICKABLE(datas->colors[i])->enable = true;
    }
    datas->pickedColorIndex = -1;
    datas->activePlayer = 0;
    datas->fighters = theFighterSystem.RetrieveAllEntityWithComponent();
    datas->players = thePlayerSystem.RetrieveAllEntityWithComponent();
    for (unsigned i=0; i<datas->fighters.size(); i++)
        PICKABLE(datas->fighters[i])->enable = true;
}

static void assignGroupIdToFighter(Entity fighter, int idx) {
    std::stringstream s;
    s << "group_" << idx;
    RENDERING(FIGHTER(fighter)->leg[0])->color =
    RENDERING(FIGHTER(fighter)->leg[1])->color =
    Color(s.str());
    std::cout << s.str() << ":" << RENDERING(FIGHTER(fighter)->leg[1])->color << std::endl;
    FIGHTER(fighter)->group = idx;
}

///----------------------------------------------------------------------------//
///--------------------- UPDATE SECTION ---------------------------------------//
///----------------------------------------------------------------------------//
void AssignColorStateManager::backgroundUpdate(float) {
}

State::Enum AssignColorStateManager::update(float dt) {
    if (datas->fighters.empty()) {
        return State::PlaceOnBattlefield;
    }
    thePickableSystem.Update(dt);

    for (int i=0; i<4; i++) {
        if (PICKABLE(datas->colors[i])->picked) {
            datas->pickedColorIndex = i;
            break;
        }
    }
    if (datas->pickedColorIndex >= 0) {
        for (unsigned i=0; i<datas->fighters.size(); i++) {
            Entity fighter = datas->fighters[i];
            if (FIGHTER(fighter)->player == datas->players[datas->activePlayer]) {
                if (PICKABLE(fighter)->picked) {
                    assignGroupIdToFighter(fighter, datas->pickedColorIndex);
                    PICKABLE(fighter)->enable = false;
                    datas->pickedColorIndex = -1;
                    datas->activePlayer = (datas->activePlayer + 1) % datas->players.size();
                    datas->fighters.erase(datas->fighters.begin() + i);
                    break;
                }
            }
        }
    }

    return State::AssignColor;
}


///----------------------------------------------------------------------------//
///--------------------- EXIT SECTION -----------------------------------------//
///----------------------------------------------------------------------------//
void AssignColorStateManager::willExit(State::Enum) {
    for (int i=0; i<4; i++) {
        RENDERING(datas->colors[i])->hide = true;
        PICKABLE(datas->colors[i])->enable = false;
    }
}

bool AssignColorStateManager::transitionCanExit(State::Enum) {
    return true;
}

void AssignColorStateManager::exit(State::Enum) {
    
}

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
#include "systems/PickableSystem.h"
#include "systems/EquipmentSystem.h"
#include "systems/SlotSystem.h"
#include <vector>

struct EquipmentStateManager::EquipmentStateManagerDatas {
    int activePlayer;
    std::vector<Entity> players, fighters, equipments;
    Entity selectedEqpt;
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
    datas->fighters = theFighterSystem.RetrieveAllEntityWithComponent();
    datas->players = thePlayerSystem.RetrieveAllEntityWithComponent();

    // Fighters
    for (unsigned i=0; i<datas->players.size(); i++) {
        int count = 0;
        for (unsigned j=0; j<datas->fighters.size(); j++) {
            if (FIGHTER(datas->fighters[j])->player == datas->players[i]) {
                int col = count / 6;
                int row = count - col * 6;
                if (i == 1) col = 7 - col;
                TRANSFORM(datas->fighters[j])->position = offset + Vector2(col * size.X, row * size.Y);
                count++;
            }
        }
    }

    // Equipment
    datas->equipments = theEquipmentSystem.RetrieveAllEntityWithComponent();
    const Vector2 minV = offset + Vector2(2 * size.X, 5 * size.Y);
    const Vector2 maxV = offset + Vector2(5 * size.X, 0);
    for (unsigned i=0; i<datas->equipments.size(); i++) {
        TRANSFORM(datas->equipments[i])->position = MathUtil::RandomVector(minV, maxV);
        TRANSFORM(datas->equipments[i])->rotation = MathUtil::RandomFloat(MathUtil::TwoPi);
        PICKABLE(datas->equipments[i])->enable = true;
    }
    datas->activePlayer = datas->selectedEqpt = 0;
}

static void assignEquipment(Entity equip, Entity slot) {
    SLOT(slot)->boundEquipment = equip;
    TRANSFORM(equip)->rotation = 0;
    TRANSFORM(equip)->parent = slot;
    TRANSFORM(equip)->position = SLOT(slot)->anchor
        - EQUIPMENT(equip)->anchor;
    PICKABLE(equip)->enable = false;
}

///----------------------------------------------------------------------------//
///--------------------- UPDATE SECTION ---------------------------------------//
///----------------------------------------------------------------------------//
void EquipmentStateManager::backgroundUpdate(float) {
}

State::Enum EquipmentStateManager::update(float dt) {
    if (datas->equipments.empty()) {
        // next state
        // todo
    } else {
        // Browse equipment. Do we have one selected ?
        for (unsigned i=0; i<datas->equipments.size(); i++) {
            if (PICKABLE(datas->equipments[i])->picked) {
                datas->selectedEqpt = datas->equipments[i];
                break;
            }
        }
        if (datas->selectedEqpt) {
            const EquipmentType::Enum type = EQUIPMENT(datas->selectedEqpt)->type;
            for (unsigned j=0; j<datas->fighters.size(); j++) {
                Entity fighter = datas->fighters[j];
                bool enablePick = false;
                
                if (FIGHTER(fighter)->player == datas->players[datas->activePlayer]) {
                    // enable if we can equip him with the selected equipment
                    switch (type) {
                        case EquipmentType::SingleHandedWeapon:
                        case EquipmentType::Shield:
                            enablePick = !SLOT(FIGHTER(fighter)->arm[0])->boundEquipment ||
                                !SLOT(FIGHTER(fighter)->arm[1])->boundEquipment;
                            break;
                        case EquipmentType::DoubleHandedWeapon:
                        case EquipmentType::Bow:
                            enablePick = !SLOT(FIGHTER(fighter)->arm[0])->boundEquipment &&
                                !SLOT(FIGHTER(fighter)->arm[1])->boundEquipment;
                            break;
                        case EquipmentType::Helmet:
                            enablePick = !SLOT(FIGHTER(fighter)->head)->boundEquipment;
                            break;
                        case EquipmentType::Armor:
                            enablePick = !SLOT(FIGHTER(fighter)->torso)->boundEquipment;
                            break;
                    }
                }
                PICKABLE(fighter)->enable = enablePick;
                if (enablePick && PICKABLE(fighter)->picked) {
                    // assign
                    EquipmentType::Enum type = EQUIPMENT(datas->selectedEqpt)->type;
                    switch (type) {
                        case EquipmentType::SingleHandedWeapon:
                        case EquipmentType::Shield:
                            if (!SLOT(FIGHTER(fighter)->arm[0])->boundEquipment) {
                                assignEquipment(datas->selectedEqpt, FIGHTER(fighter)->arm[0]);
                            } else {
                                assignEquipment(datas->selectedEqpt, FIGHTER(fighter)->arm[1]);
                            }
                            break;
                        case EquipmentType::DoubleHandedWeapon:
                        case EquipmentType::Bow:
                            assignEquipment(datas->selectedEqpt, FIGHTER(fighter)->arm[0]);
                            assignEquipment(datas->selectedEqpt, FIGHTER(fighter)->arm[1]);
                            break;
                        case EquipmentType::Helmet:
                            assignEquipment(datas->selectedEqpt, FIGHTER(fighter)->head);
                            break;
                        case EquipmentType::Armor:
                            assignEquipment(datas->selectedEqpt, FIGHTER(fighter)->torso);
                            break;
                    }
                    datas->selectedEqpt = 0;
                    datas->activePlayer = (datas->activePlayer + 1) % datas->players.size();
                }
            }
        } else {
            for (unsigned j=0; j<datas->fighters.size(); j++) {
                PICKABLE(datas->fighters[j])->enable = false;
            }
        }
    }

    thePickableSystem.Update(dt);

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

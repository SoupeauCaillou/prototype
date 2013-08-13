#pragma once

#include "systems/System.h"

namespace EAction {
    enum Enum {
        None,
        SelectedCell
    };
}

struct ActionComponent {
    ActionComponent() : action(EAction::None) {
        memset(&SelectedCellParams, 0, sizeof(SelectedCellParams));
    }

    EAction::Enum action;
    struct {
        Entity cell;
        Entity player;
    } SelectedCellParams;
};

#define theActionSystem ActionSystem::GetInstance()
#define ACTION(e) theActionSystem.Get(e)

UPDATABLE_SYSTEM(Action)
};

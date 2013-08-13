#pragma once

#include "systems/System.h"

namespace EAction {
    enum Enum {
        None,
        ClickButton
    };
}

struct ActionComponent {
    ActionComponent() : action(EAction::None) {
        memset(&ClickButtonParams, 0, sizeof(ClickButtonParams));
    }

    EAction::Enum action;
    struct {
        Entity button;
        Color color;
    } ClickButtonParams;
};

#define theActionSystem ActionSystem::GetInstance()
#define ACTION(e) theActionSystem.Get(e)

UPDATABLE_SYSTEM(Action)
};

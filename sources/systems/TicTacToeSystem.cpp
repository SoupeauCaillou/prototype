#include "TicTacToeSystem.h"

#include "systems/TransformationSystem.h"

#include "systems/AnchorSystem.h"
#include "systems/RenderingSystem.h"
#include "util/IntersectionUtil.h"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/compatibility.hpp>

INSTANCE_IMPL(TicTacToeSystem);

TicTacToeSystem::TicTacToeSystem() : ComponentSystemImpl <TicTacToeComponent>("TicTacToe") {
    TicTacToeComponent tttc;
    // componentSerializer.add(new Property<int>("action", OFFSET(action, ac), 0));
}

void TicTacToeSystem::DoUpdate(float) {
    for (auto it: components) {
        const Entity e = it.first;
        auto* tttc = it.second;
    }
}

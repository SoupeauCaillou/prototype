/*
    This file is part of Prototype.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Jordane Pelloux-Prayer

    Prototype is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Prototype is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Prototype.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "TacticalAISystem.h"
#include "systems/SoldierSystem.h"
#include "systems/MemorySystem.h"
#include "systems/ActionSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/VisionSystem.h"
#include "systems/TransformationSystem.h"
#include "PrototypeGame.h"

struct IterativeGameState {
    // State we came from
    std::shared_ptr<IterativeGameState> parent;
    // Action that transformed parent -> us
    ActionComponent actionFromParent;

    ///// State variables, used for state evaluation
    GridPos myPosition;
    SoldierComponent mySoldier;

    std::vector<Entity> enemies;
    std::vector<GridPos> enemyPositions;
    std::vector<SoldierComponent> enemySoldier;
};
static void bpa(PrototypeGame* game, std::shared_ptr<IterativeGameState> current, int actionPointsLeft, int* bestScore, std::shared_ptr<IterativeGameState>& best);

INSTANCE_IMPL(TacticalAISystem);

TacticalAISystem::TacticalAISystem() : ComponentSystemImpl<TacticalAIComponent>("TacticalAI") {
	TacticalAIComponent sc;

}

void TacticalAISystem::DoUpdate(float) {
    for (auto p: components) {
        const Entity e = p.first;
        auto* tactical = p.second;
        auto* memory = MEMORY(e);

        if (memory->enemyLastKnownPos.empty())
            continue;

        // for each of my soldiers
        theSoldierSystem.forEachECDo([this, e, memory] (Entity entity, SoldierComponent* sc) -> void {
            if (sc->player != e)
                return;
            if (!VISION(entity)->enabled)
                return;
            auto* state = new IterativeGameState();
            state->myPosition = game->grid.positionToGridPos(TRANSFORM(entity)->position);
            state->mySoldier = *sc;

            for (auto p: memory->enemyLastKnownPos) {
                Entity enemy = p.first;
                state->enemies.push_back(enemy);
                state->enemyPositions.push_back(game->grid.positionToGridPos(TRANSFORM(enemy)->position));
                state->enemySoldier.push_back(*SOLDIER(enemy));
            }

            std::shared_ptr<IterativeGameState> sptr(state), best;
            int bestScore = 0;
            bpa(game,
                sptr,
                glm::min(sc->actionPointsLeft, PLAYER(e)->actionPointsLeft),
                &bestScore,
                best);
            LOGI("bpa done for " << theEntityManager.entityName(entity) << ": bestScore = " << bestScore);
        });
    }
}

static int evaluate(IterativeGameState* state) {
    return 0;
}

static void bpa(PrototypeGame* game, std::shared_ptr<IterativeGameState> current, int actionPointsLeft, int* bestScore, std::shared_ptr<IterativeGameState>& best) {
    // evaluate current state
    int score = evaluate(current.get());
    if (score > *bestScore) {
        *bestScore = score;
        best = current;
    }

    // Recurse over all potential move from there
    if (actionPointsLeft >= ActionSystem::ActionCost(Action::MoveTo)) {
        std::vector<GridPos> neighbors = game->grid.getNeighbors(current->myPosition, false);
        for (auto& gp: neighbors) {
            if (!game->grid.isPathBlockedAt(gp)) {
                std::shared_ptr<IterativeGameState> move (new IterativeGameState(*current));
                move->parent = current;
                move->myPosition = gp;
                move->actionFromParent.type = Action::MoveTo;
                move->actionFromParent.moveToTarget = game->grid.gridPosToPosition(gp);

                // Recursive
                bpa(game, move, actionPointsLeft - ActionSystem::ActionCost(Action::MoveTo), bestScore, best);
            }
        }
    }
    // Recurse over all potential shots from here
    if (actionPointsLeft >= ActionSystem::ActionCost(Action::Attack)) {
        // browse enemies
        for (unsigned i=0; i<current->enemies.size(); i++) {
            // is this enemy alive ?
            if (current->enemySoldier[i].maxActionPointsPerTurn > 0) {
                // is this enemy reachable
                int line = game->grid.canDrawLine(current->myPosition, current->enemyPositions[i]);

                if (line <= current->mySoldier.attackRange.t2) {
                    std::shared_ptr<IterativeGameState> atk (new IterativeGameState(*current));
                    atk->parent = current;
                    atk->actionFromParent.type = Action::Attack;
                    atk->actionFromParent.attackTarget = current->enemies[i];
                    atk->enemySoldier[i].maxActionPointsPerTurn -= current->mySoldier.attackDamage
                        * ActionSystem::DetermineHitProbability(
                            current->mySoldier,
                            current->myPosition,
                            current->enemyPositions[i]);

                    // Recursive
                    bpa(game, atk, actionPointsLeft - ActionSystem::ActionCost(Action::Attack), bestScore, best);
                }
            }
        }
    }
}

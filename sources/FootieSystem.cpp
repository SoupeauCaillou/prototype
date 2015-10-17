/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "FootieSystem.h"
#include "util/SerializerProperty.h"


#include "systems/AnimationSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"

#include "util/IntersectionUtil.h"

INSTANCE_IMPL(FootieSystem);

FootieSystem::FootieSystem() : ComponentSystemImpl<FootieComponent>(HASH("Footie", 0xbfb81f4c)) {

}

static bool canChangeAction(Entity p) {
    float can = 0;
    if (theAnimationSystem.queryAttributes(
        ANIMATION(p),
        HASH("can_change_action", 0xef3afa0c),
        &can,
        1)) {
        return can > 0;
    } else {
        return false;
    }
}

void FootieSystem::DoUpdate(float dt) {
    /** XXX: should probably be a 2 step update.
     *  - #1: update as if each player is alone (kick the ball, move anywhere)
     *  - #2: resolve conflicts (B tackled the ball that A intended to shot etc)
     */

     /** XXX: remember intended action (tackle) if it cannot be performed atm
      *  (wrong anim frame)
      */
    FOR_EACH_ENTITY_COMPONENT(Footie, e, comp)
        float runningSpeed = 0.0f;

        actions::Enum nextAction = actions::Idle;

        glm::vec2 dir(comp->input.direction);
        float dirLength = glm::length(dir);

        if (dirLength > 0.1) {
            nextAction = (comp->input.accums[buttons::Sprint] > 0) ? actions::Run : actions::Walk;
        } else {
            nextAction = actions::Idle;
        }

        if (comp->input.released[buttons::Tackle]) {
            nextAction = actions::Tackle;
        }

        // Allow new actions only at certain key-frames of animations
        if (!canChangeAction(e)) {
            // prevent direction change during non-running actions
            if (comp->state.currentAction != actions::Run && comp->state.currentAction != actions::Walk) {
                dir = comp->state.previousDir;
                dirLength = glm::length(dir);
            }
        } else {
            comp->state.currentAction = nextAction;
        }

        switch (comp->state.currentAction) {
            case actions::Idle:
                runningSpeed = 0.0f;
                break;
            case actions::Walk:
                runningSpeed = game->tuning.f(HASH("walking_speed", 0x73dcc3ec));
                break;
            case actions::Run:
                runningSpeed = game->tuning.f(HASH("running_speed", 0x1f34eb01));
                break;
            case actions::Tackle:
                runningSpeed = game->tuning.f(HASH("tackle_speed", 0xf53acc8e));
                dir = comp->state.previousDir;
                break;
        }
        ANIMATION(e)->name = game->teamAnimations[comp->team][comp->state.currentAction];

        bool touchBall = IntersectionUtil::rectangleRectangle(TRANSFORM(game->ball), TRANSFORM(comp->hitzone));

        // kick the ball
        if ((comp->state.currentAction == actions::Run || comp->state.currentAction == actions::Walk) && dirLength > 0) {
            const float kickForce = game->tuning.f(HASH("kick_force_slow", 0x3987715d));

            static bool kickEnabled = true;

            // only kick if ball is going slower and/or in the wrong direction
            if (kickEnabled) {
                if (touchBall) {
                    glm::vec2 desiredBallPosition =
                        TRANSFORM(comp->hitzone)->position +
                        dir * runningSpeed * game->tuning.f(HASH("ball_advance_lookup_sec", 0xcf261ebb));

                    glm::vec2 diff = desiredBallPosition - TRANSFORM(game->ball)->position;

                    PHYSICS(game->ball)->linearVelocity = glm::vec2(0.0f);
                    PHYSICS(game->ball)->addForce(Force(glm::length(diff) * kickForce * dir, glm::vec2(0.0f)), 0.016f);
                    kickEnabled = false;

                    game->lastPlayerWhoKickedTheBall = e;
                }
            } else {
                if (!touchBall) {
                    kickEnabled = true;
                }
            }
            RENDERING(comp->hitzone)->color = Color(!kickEnabled, 0, kickEnabled);
        }
        if (game->lastPlayerWhoKickedTheBall == e &&
            comp->input.released[buttons::AbandonBall] > 0) {
            game->lastPlayerWhoKickedTheBall = 0;
        }
        if (runningSpeed > 0) {
            if (game->lastPlayerWhoKickedTheBall == e) {
                // lock direction until contact
                if (!touchBall) {
                    dir = glm::normalize(TRANSFORM(game->ball)->position - TRANSFORM(comp->hitzone)->position);
                }
            }

            if (dir.x || dir.y) {
                TRANSFORM(comp->hitzone)->position += runningSpeed * dt * glm::normalize(dir) * glm::min(dirLength, 1.0f);
            }

            if (dir.x < 0) {
                RENDERING(e)->flags |= RenderingFlags::MirrorHorizontal;
            } else {
                RENDERING(e)->flags &= ~RenderingFlags::MirrorHorizontal;
            }
        }

        comp->state.previousDir = dir;

        if (comp->input.released[buttons::Pass] > 0) {
            /* XXX todo */
            #if 0
            // change active player
            float minDistToTheBall = 100000000;
            int idx = 0;
            for (int i = 0; i < players.size(); i++) {
                const auto& p = players[i];
                if (p.render == player.render) {
                    continue;
                }
                if (p.joystick >= 0) {
                    continue;
                }
                float dist = glm::distance(TRANSFORM(p.hitzone)->position, TRANSFORM(ball)->position);
                if (dist < minDistToTheBall) {
                    minDistToTheBall = dist;
                    idx = i;
                }
            }
            players[idx].joystick = player.joystick;
            player.joystick = -1;
            #endif
        }
    }
}


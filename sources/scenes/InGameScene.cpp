/*
    This file is part of Prototype.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

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

#include "base/SceneState.h"

#include "PrototypeGame.h"
#include "systems/RenderingSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/AutonomousAgentSystem.h"
#include "util/IntersectionUtil.h"
#include "util/Random.h"
#include "base/EntityManager.h"

#include "PlayerSystem.h"
#include "EquipmentSystem.h"
#include "SwordSystem.h"
#include "HealthSystem.h"
#include "GunSystem.h"
#include "IASystem.h"
#include <algorithm>

struct InGameScene : public SceneState<Scene::Enum> {
    PrototypeGame* game;

    InGameScene(PrototypeGame* game)
        : SceneState<Scene::Enum>("in_game",
                                  SceneEntityMode::DoNothing,
                                  SceneEntityMode::InstantaneousOnPreExit) {
        this->game = game;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION
    ///----------------------------------------//
    ///----------------------------------------------------------------------------//

    int enemySpawned;
    float timeElapsed;
    std::vector<Entity> aliveEnemies;
    void onEnter(Scene::Enum) override {
        // RENDERING(game->guy[0])->show = true;
        enemySpawned = 0;
        timeElapsed = 0;
        batch.enable(ActivationMode::Instantaneous);
        HEALTH(game->guy[0])->currentHP = 1;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION
    ///---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        TWEAK(int, totalEnemyCount) = 100;
        TWEAK(float, totalTime) = 600;

        timeElapsed += dt;
        if (timeElapsed > totalTime) {
            LOGI("Defeat");
            return Scene::Menu;
        }

        if (aliveEnemies.empty() && enemySpawned == totalEnemyCount) {
            LOGI("Victory!");
            return Scene::Menu;
        }


        if (enemySpawned < totalEnemyCount) {
            for (int i=0; i<4; i++) {
                if (PLAYER(game->guy[i])->input.actions[1] == InputState::Released) {
                    glm::vec2 spawnPosition[] = {
                        glm::vec2(7, 0.0f),
                        glm::vec2(-7, 0.0f),
                        glm::vec2(0.0f, 7.0f),
                        glm::vec2(0.0f, -7.0f),
                    };
                    int nextSpawn = Random::Int(0, 3);


                    // spawn enemy
                    Entity enemy = theEntityManager.CreateEntityFromTemplate("enemy");

                    nextSpawn = (nextSpawn + 1) % 4;
                    TRANSFORM(enemy)->position = spawnPosition[nextSpawn];
                    TRANSFORM(enemy)->rotation = glm::pi<float>()+atan2(TRANSFORM(enemy)->position.y,TRANSFORM(enemy)->position.x);

                    EQUIPMENT(enemy)->hand.right = theEntityManager.CreateEntityFromTemplate("gun");
                    IA(enemy)->state = IAState::EnterArena;
                    aliveEnemies.push_back(enemy);
                    enemySpawned++;

                    #if 0
                    TWEAK(float, maxSpeed) = 5;
                    AUTONOMOUS(enemy)->maxSpeed = maxSpeed;
                    TWEAK(float, maxForce) = 20;
                    AUTONOMOUS(enemy)->maxForce = maxForce;
                    AUTONOMOUS(enemy)->flee.radius = 3;
                    AUTONOMOUS(enemy)->flee.target = game->guy[0];
                    AUTONOMOUS(enemy)->limit.limit = game->battleground;
                    TWEAK(float, epsilon) = 0.3;
                    AUTONOMOUS(enemy)->limit.epsilon = epsilon;
                    AUTONOMOUS(enemy)->dangerThreshold = 1.0f;
                    #endif
                }
                break;
            }
        }

        {
            auto* tc = TRANSFORM(e(HASH("in_game/time_left", 0xabe1c4d5)));
            tc->size.x = TRANSFORM(game->camera)->size.x * (timeElapsed / totalTime);
            tc->position.x = (-TRANSFORM(game->camera)->size.x + tc->size.x) * 0.5f;
        }
        {
            auto* tc = TRANSFORM(e(HASH("in_game/enemy_spawned", 0xa69908ad)));
            tc->size.x = TRANSFORM(game->camera)->size.x * (enemySpawned / (float)totalEnemyCount);
            tc->position.x = (-TRANSFORM(game->camera)->size.x + tc->size.x) * 0.5f;
        }

        theEquipmentSystem.Update(dt);
        theSwordSystem.Update(dt);
        thePlayerSystem.Update(dt);
        theGunSystem.Update(dt);
        theIASystem.Update(dt);

        // fix entities position
        {
            TWEAK(float, limitX) = 0.5;
            TWEAK(float, limitY) = 0.5;
            TWEAK(float, epsilon) = 0.75;

            float absLimitX = limitX * TRANSFORM(game->battleground)->size.x;
            float absLimitY = limitY * TRANSFORM(game->battleground)->size.y;
            for (int i=0; i<4; i++) {
                auto* tc = TRANSFORM(game->guy[i]);
                float size = tc->size.x * epsilon;

                if ((tc->position.x + size) > absLimitX) {
                    tc->position.x = absLimitX - size;
                } else if ((tc->position.x - size) < -absLimitX) {
                    tc->position.x = -absLimitX + size;
                }
                if ((tc->position.y + size) > absLimitY) {
                    tc->position.y = absLimitY - size;
                } else if ((tc->position.y - size) < -absLimitY) {
                    tc->position.y = -absLimitY + size;
                }
                break;
            }
        }

        const std::vector<Entity>& players = thePlayerSystem.RetrieveAllEntityWithComponent();
        {
            TWEAK(float, forceAmplitude) = 1500.0f;
            TWEAK(float, dispersion) = 0.6f;
            TWEAK(float, partScale) = 5;
            std::vector<Entity> toRemove;
            for (auto enemy: players) {

                if (HEALTH(enemy)->currentHP <= 0) {
                    Entity hitBy = HEALTH(enemy)->hitBy;

                    /* split body */
                    for (int i=0; i<9; i++) {
                        glm::vec2 baseDirection =
                            glm::rotate(
                                glm::vec2(1.0f, 0.0f),
                                TRANSFORM(hitBy)->rotation + glm::pi<float>() * 0.5f +
                                Random::Float(-dispersion, dispersion));
                        Entity part = theEntityManager.CreateEntityFromTemplate("enemy_part");
                        TRANSFORM(part)->size *= Random::Float(1, partScale);
                        TRANSFORM(part)->position =
                            TRANSFORM(enemy)->position +
                            TRANSFORM(enemy)->size *
                            glm::vec2(Random::Float(-0.4, 0.4), Random::Float(-0.4, 0.4));
                        PHYSICS(part)->addForce(
                            baseDirection * forceAmplitude * Random::Float(0.2, 1.0f),
                            //glm::rotate(glm::vec2(forceAmplitude, 0.0f), Random::Float(0, 6.2)),
                            glm::vec2(0.0f),
                            dt);
                        if (enemy == game->guy[0]) {
                            RENDERING(part)->color = RENDERING(game->guy[0])->color;
                        }
                    }
                    /* disperse weapons */
                    if (enemy != game->guy[0]) {
                        for (int i=0; i<2; i++) {
                            Entity eq = EQUIPMENT(enemy)->hands[i];
                            if (eq) {
                                TRANSFORM(eq)->z = 0.2;
                                glm::vec2 baseDirection =
                                    glm::rotate(
                                        glm::vec2(1.0f, 0.0f),
                                        TRANSFORM(hitBy)->rotation + glm::pi<float>() * 0.5f +
                                        Random::Float(-dispersion, dispersion));

                                /* move */
                                PHYSICS(eq)->mass = 1;
                                PHYSICS(eq)->addForce(
                                    baseDirection * forceAmplitude * Random::Float(0.2, 1.0f),
                                    //glm::rotate(glm::vec2(forceAmplitude, 0.0f), Random::Float(0, 6.2)),
                                    glm::vec2(0.0f),
                                    dt);
                            }
                        }
                    }

                    if (COLLISION(hitBy)->group == 3) {
                        theEntityManager.DeleteEntity(hitBy);
                    }
                    LOGI("killed " << enemy);
                    // hackish
                    if (enemy != game->guy[0]) {
                        deletePlayer(enemy);
                        toRemove.push_back(enemy);
                    }
                } else if (0){
                    if (enemy != game->guy[0]) {
                        glm::vec2 diff = TRANSFORM(game->guy[0])->position - TRANSFORM(enemy)->position;
                        TRANSFORM(enemy)->rotation = atan2(diff.y, diff.x);
                    }
                }
            }
            auto newEnd = std::remove_if(aliveEnemies.begin(),
                            aliveEnemies.end(),
                            [&toRemove] (Entity e) { return std::find(toRemove.begin(), toRemove.end(), e) != toRemove.end(); });
            aliveEnemies.erase(newEnd, aliveEnemies.end());
        }

        // check life
        for (int i=0; i<4; i++) {
            if (HEALTH(game->guy[i])->currentHP <= 0) {
                LOGI("You lost");
                return Scene::Menu;
            }
            break;
        }


        return Scene::InGame;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION
    ///-----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onExit(Scene::Enum) override {
        for (auto e: aliveEnemies) {
            deletePlayerAndWeapons(e);
        }
        aliveEnemies.clear();
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateInGameSceneHandler(PrototypeGame* game) {
        return new InGameScene(game);
    }
}

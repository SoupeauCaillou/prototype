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
#include "base/StateMachine.h"

#include "Scenes.h"

#include "systems/RocketSystem.h"

#include "systems/ButtonSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/TransformationSystem.h"

struct ScoreScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    // Scene variables
    Entity replayButton, back2Menu, scoreText;

    ScoreScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    // Destructor
    ~ScoreScene() {}

    // Setup internal var, states, ...
    void setup() override {
        replayButton = theEntityManager.CreateEntity("replayButton",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("replayButton"));
        back2Menu = theEntityManager.CreateEntity("back2Menu",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("back2Menu"));
        scoreText = theEntityManager.CreateEntity("scoreText",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("scoreText"));
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreEnter(Scene::Enum) {}
    bool updatePreEnter(Scene::Enum, float) override {return true;}
    void onEnter(Scene::Enum) override {
        RENDERING(replayButton)->show =
            RENDERING(back2Menu)->show =
            TEXT_RENDERING(scoreText)->show = true;
        BUTTON(replayButton)->enabled =
            BUTTON(back2Menu)->enabled = true;
        std::stringstream a;
        FOR_EACH_ENTITY(Rocket, e)
            a << TRANSFORM(e)->position.y;
        }
        TEXT_RENDERING(scoreText)->text = a.str();
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override { 
        if (BUTTON(replayButton)->clicked)
            return Scene::Loading;
        if (BUTTON(back2Menu)->clicked)
            return Scene::Menu;
        return Scene::Score;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {}
    bool updatePreExit(Scene::Enum, float) override {return true;}
    void onExit(Scene::Enum) override {
        RENDERING(replayButton)->show =
            RENDERING(back2Menu)->show =
            TEXT_RENDERING(scoreText)->show = false;
        BUTTON(replayButton)->enabled =
            BUTTON(back2Menu)->enabled = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateScoreSceneHandler(PrototypeGame* game) {
        return new ScoreScene(game);
    }
}

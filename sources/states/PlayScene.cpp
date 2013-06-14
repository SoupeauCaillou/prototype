/*
	This file is part of RecursiveRunner.

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
#include "base/StateMachine.h"

#include "Scenes.h"

#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"

#include "util/IntersectionUtil.h"
#include "util/DrawSomething.h"
#include "util/Grid.h"

#include "systems/ADSRSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/BlockSystem.h"
#include "systems/SpotSystem.h"
#include "systems/TransformationSystem.h"

#include "PrototypeGame.h"

#include <glm/gtx/vector_angle.hpp>
#include <iomanip>


struct PlayScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    Entity objectiveProgression, victory, fadeout;

    PlayScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        glm::vec2 pointOfView(0.6222f, 0.0778f);
        // bug dans glm? notre point est à gauche de "l'origine", et un peu plus bas ... donc l'angle devrait etre -179.9 pas 179.9
        LOGF_IF(0.f <  glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(glm::vec2(-4.0966f, 0.0701f ) - pointOfView)),
            "Bug with GLM. You should fix it in glm/gtx/vector_angle.inl:36 -> change epsilon value to 0.00001 ");



        objectiveProgression = theEntityManager.CreateEntity("objective",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_number"));
        TRANSFORM(objectiveProgression)->position = glm::vec2(7, 6);
        TEXT_RENDERING(objectiveProgression)->text = "0.0\%";

        victory = theEntityManager.CreateEntity("Victory",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("grid_number"));
        TEXT_RENDERING(victory)->text = "Victory!";
        TRANSFORM(victory)->position = glm::vec2(0., PlacementHelper::ScreenHeight / 4.);
        TRANSFORM(victory)->z = .95;

        fadeout = theEntityManager.CreateEntity("playscene fade out");
        ADD_COMPONENT(fadeout, Rendering);
        RENDERING(fadeout)->color = Color(0., 0., 0.);
        ADD_COMPONENT(fadeout, ADSR);
        ADSR(fadeout)->attackMode = Mode::Quadratic;
        ADSR(fadeout)->attackTiming = 5.;
        ADSR(fadeout)->sustainValue = 1.;
        ADSR(fadeout)->decayTiming = ADSR(fadeout)->releaseTiming = 0.;
        ADD_COMPONENT(fadeout, Transformation);
        TRANSFORM(fadeout)->size = glm::vec2(PlacementHelper::ScreenWidth, PlacementHelper::ScreenHeight);
        TRANSFORM(fadeout)->z = 0.93;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
#if SAC_DEBUG
        Grid::EnableGrid();
#endif

        TEXT_RENDERING(objectiveProgression)->show = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float dt) override {
        theBlockSystem.Update(dt);
        theSpotSystem.Update(dt);

        //update objective
        {
            //update the text from the entity
            std::stringstream a;
            a << std::fixed << std::setprecision(2) << theSpotSystem.totalHighlightedDistance2Done << "/ "
            << std::fixed << std::setprecision(2) << theSpotSystem.totalHighlightedDistance2Objective << " %";

            TEXT_RENDERING(objectiveProgression)->text = a.str();
        }

        //if objective is done or right click, go back to menu
        if (theTouchInputManager.isTouched(1)
            || theSpotSystem.totalHighlightedDistance2Objective - theSpotSystem.totalHighlightedDistance2Done < 0.001) {
            RENDERING(fadeout)->show = TEXT_RENDERING(victory)->show = true;
            return Scene::Menu;
        }

        return Scene::Play;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    bool updatePreExit(Scene::Enum , float ) {
        float value = ADSR(fadeout)->value;
        TEXT_RENDERING(victory)->charHeight = 3 * value;
        RENDERING(fadeout)->color.a = value;
        return (theTouchInputManager.isTouched(0) && value == ADSR(fadeout)->sustainValue);
    }

    void onPreExit(Scene::Enum) override {
        ADSR(fadeout)->active = RENDERING(fadeout)->show = TEXT_RENDERING(victory)->show = true;
    }

    void onExit(Scene::Enum) override {
        Draw::DrawPointRestart("SpotSystem");
        Draw::DrawVec2Restart("SpotSystem");
        Draw::DrawTriangleRestart("SpotSystem");

        FOR_EACH_ENTITY(Spot, e)
            theEntityManager.DeleteEntity(e);
        }
        FOR_EACH_ENTITY(Block, e)
            theEntityManager.DeleteEntity(e);
        }

#if SAC_DEBUG
        Grid::DisableGrid();
#endif
        ADSR(fadeout)->active =
        RENDERING(fadeout)->show =
        TEXT_RENDERING(victory)->show =
        TEXT_RENDERING(objectiveProgression)->show = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreatePlaySceneHandler(PrototypeGame* game) {
        return new PlayScene(game);
    }
}

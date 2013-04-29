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

#include "systems/TransformationSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/GraphSystem.h"

#include "PrototypeGame.h"
#include "api/CommunicationAPI.h"
#include "api/StorageAPI.h"

#include "util/ScoreStorageProxy.h"

struct SocialCenterScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    Entity graph;

    Entity menuBtn;

    SocialCenterScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        menuBtn = theEntityManager.CreateEntity("social_menuBtn");
        ADD_COMPONENT(menuBtn, Transformation);
        TRANSFORM(menuBtn)->z = .9;
        TRANSFORM(menuBtn)->position = glm::vec2(9., -5);
        ADD_COMPONENT(menuBtn, Button);
        ADD_COMPONENT(menuBtn, Rendering);
        RENDERING(menuBtn)->color = Color::random();

        graph = theEntityManager.CreateEntity("social_graph");
        ADD_COMPONENT(graph, Transformation);
        TRANSFORM(graph)->z = .9;
        TRANSFORM(graph)->size = glm::vec2(10,10);
        TRANSFORM(graph)->position = glm::vec2(0., 0);

        ADD_COMPONENT(graph, Rendering);

        ADD_COMPONENT(graph, TextRendering);
        TEXT_RENDERING(graph)->positioning = TextRenderingComponent::LEFT;
        TEXT_RENDERING(graph)->maxCharHeight = 0.4;
        TEXT_RENDERING(graph)->text = "Scores graphic";

        ADD_COMPONENT(graph, Graph);
        RENDERING(graph)->texture = theRenderingSystem.loadTextureFile("__scores_graph");

        GRAPH(graph)->textureName = "__scores_graph";
        GRAPH(graph)->lineColor = Color(1., 0., 0.);
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreEnter(Scene::Enum) override {
        ScoreStorageProxy ssp;
        game->gameThreadContext->storageAPI->loadEntries(&ssp, "*", "");
        int count = -1;

        //clear the graph before redrawing
        GRAPH(graph)->pointsList.clear();

        while (! ssp.isEmpty()) {
            Score score = ssp._queue.front();
            LOGI("one more score: " << score.time);
            GRAPH(graph)->pointsList.push_back(std::pair<int, float>(++count, score.time));
            ssp.popAnElement();
        }
        GRAPH(graph)->reloadTexture = true;
    }

    void onEnter(Scene::Enum) override {
        BUTTON(menuBtn)->enabled =
        TEXT_RENDERING(graph)->show =
        RENDERING(graph)->show =
        RENDERING(menuBtn)->show = true;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override {
        if (BUTTON(menuBtn)->clicked)
            return Scene::Menu;
        return Scene::SocialCenter;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onExit(Scene::Enum) override {
        BUTTON(menuBtn)->enabled =
        TEXT_RENDERING(graph)->show =
        RENDERING(graph)->show =
        RENDERING(menuBtn)->show = false;
    }
};


namespace Scene {
    StateHandler<Scene::Enum>* CreateSocialCenterSceneHandler(PrototypeGame* game) {
        return new SocialCenterScene(game);
    }
}

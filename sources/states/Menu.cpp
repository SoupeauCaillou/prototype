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
#include "StateManager.h"

#include <sstream>
#include <vector>
#include <iomanip>

#include <base/EntityManager.h>

#include <systems/TransformationSystem.h>
#include <systems/ButtonSystem.h>
#include <systems/RenderingSystem.h>
#include <systems/TextRenderingSystem.h>
#include <systems/AutoDestroySystem.h>
#include <systems/PhysicsSystem.h>

#include "api/StorageAPI.h"
#include "util/ScoreStorageProxy.h"

#include "PrototypeGame.h"

struct MenuState::MenuStateDatas {
    Entity socialBtn, timer;
    float timeElapsed;
};

MenuState::MenuState(PrototypeGame* game) : StateManager(State::Menu, game) {
    datas = new MenuStateDatas;
}

MenuState::~MenuState() {
    delete datas;
}

void MenuState::setup() {
    datas->timeElapsed = 0.f;

    Entity socialBtn = datas->socialBtn = theEntityManager.CreateEntity("socialBtn");
    ADD_COMPONENT(socialBtn, Transformation);
    TRANSFORM(socialBtn)->z = .9;
    TRANSFORM(socialBtn)->position = glm::vec2(9., -5);
    ADD_COMPONENT(socialBtn, Button);
    ADD_COMPONENT(socialBtn, Rendering);
    RENDERING(socialBtn)->color = Color::random();


    Entity timer = datas->timer = theEntityManager.CreateEntity("timer");
    ADD_COMPONENT(timer, Transformation);
    TRANSFORM(timer)->z = .9;
    TRANSFORM(timer)->position = glm::vec2(9., -5);
    ADD_COMPONENT(timer, TextRendering);
    TEXT_RENDERING(timer)->show = true;
    TEXT_RENDERING(timer)->text = "0";
    TEXT_RENDERING(timer)->charHeight = 2;
    TEXT_RENDERING(timer)->cameraBitMask = 0xffff;
    TEXT_RENDERING(timer)->positioning = TextRenderingComponent::RIGHT;

}



///----------------------------------------------------------------------------//
///--------------------- ENTER SECTION ----------------------------------------//
///----------------------------------------------------------------------------//
void MenuState::willEnter(State::Enum) {
}

bool MenuState::transitionCanEnter(State::Enum) {
    return true;
}


void MenuState::enter(State::Enum) {
    TEXT_RENDERING(datas->timer)->show =
    BUTTON(datas->socialBtn)->enabled =
    RENDERING(datas->socialBtn)->show = true;
}


///----------------------------------------------------------------------------//
///--------------------- UPDATE SECTION ---------------------------------------//
///----------------------------------------------------------------------------//
void MenuState::backgroundUpdate(float) {
}

State::Enum MenuState::update(float dt) {
    //update the timer
    {
    float & timeElapsed = datas->timeElapsed;

    timeElapsed += dt;

    //update the text from the entity
    std::stringstream a;
    a << game->gameThreadContext->localizeAPI->text("time") << ": " <<
        std::fixed << std::setprecision(2) << timeElapsed << " s";
    TEXT_RENDERING(datas->timer)->text = a.str();
    }

    {
        //static int i=0;
        //LOGV(1, "Nombre d'entité = " << ++i);

        Entity eq = theEntityManager.CreateEntity();
        ADD_COMPONENT(eq, Transformation);
        TRANSFORM(eq)->z = 0.5;
        TRANSFORM(eq)->size = glm::vec2(0.5,0.5);
        TRANSFORM(eq)->position = glm::vec2(glm::linearRand(-10.0f, 10.0f), glm::linearRand(-10.0f, 10.0f));
        ADD_COMPONENT(eq, Rendering);
        RENDERING(eq)->color = Color::random();
        RENDERING(eq)->show = true;
        RENDERING(eq)->cameraBitMask = 0xffff;
        ADD_COMPONENT(eq, Physics);
        PHYSICS(eq)->mass = 1;
        PHYSICS(eq)->gravity = glm::vec2(0, -1);

        ADD_COMPONENT(eq, AutoDestroy);
        AUTO_DESTROY(eq)->type = AutoDestroyComponent::OUT_OF_AREA;
        AUTO_DESTROY(eq)->params.area.x = AUTO_DESTROY(eq)->params.area.y = 0;
        AUTO_DESTROY(eq)->params.area.w = TRANSFORM(game->camera)->size.x;
        AUTO_DESTROY(eq)->params.area.h = TRANSFORM(game->camera)->size.y;
    }

   if (BUTTON(datas->socialBtn)->clicked)
        return State::SocialCenter;

    return State::Menu;
}


///----------------------------------------------------------------------------//
///--------------------- EXIT SECTION -----------------------------------------//
///----------------------------------------------------------------------------//
void MenuState::willExit(State::Enum) {
    ScoreStorageProxy ssp;
    Score s;
    s.points = datas->timeElapsed;
    ssp._queue.push(s);
    game->gameThreadContext->storageAPI->saveEntries((IStorageProxy*)&ssp);
}

bool MenuState::transitionCanExit(State::Enum) {
    return true;
}

void MenuState::exit(State::Enum) {
    TEXT_RENDERING(datas->timer)->show =
    BUTTON(datas->socialBtn)->enabled =
    RENDERING(datas->socialBtn)->show = false;
}

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

#include "base/EntityManager.h"

#include "systems/RenderingSystem.h"

struct UserInputScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;

    // Scene variables
    std::list<Entity> walls;
    std::list<Entity> yEnnemies;
    std::list<Entity> bEnnemies;
    std::list<Entity> players;
    std::list<Entity> objs;
    Entity background;

    UserInputScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    // Destructor
    ~UserInputScene() {}

    // Setup internal var, states, ...
    void setup() override {
        std::stringstream a;
        for (int i=1; i<27; ++i) {
            a.str("");
            a << "wall_" << i;
            Entity wall = theEntityManager.CreateEntity(a.str(),
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(a.str()));
            walls.push_back(wall);
        }

        for (int i=1; i<10; ++i) {
            a.str("");
            a << "yellowSoldier_" << i;
            Entity s = theEntityManager.CreateEntity(a.str(),
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(a.str()));
            walls.push_back(s);
        }

        for (int i=1; i<3; ++i) {
            a.str("");
            a << "blueSoldier_" << i;
            Entity s = theEntityManager.CreateEntity(a.str(),
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(a.str()));
            bEnnemies.push_back(s);
        }


        for (int i=1; i<3; ++i) {
            a.str("");
            a << "objective_" << i;
            Entity s = theEntityManager.CreateEntity(a.str(),
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load(a.str()));
            objs.push_back(s);
        }

        players.push_back(theEntityManager.CreateEntity("playerb",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("playerb")));
        players.push_back(theEntityManager.CreateEntity("playerg",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("playerg")));
        players.push_back(theEntityManager.CreateEntity("playerr",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("playerr")));
        players.push_back(theEntityManager.CreateEntity("playery",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("playery")));

        background = theEntityManager.CreateEntity("background",
            EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("background"));
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreEnter(Scene::Enum) {}
    bool updatePreEnter(Scene::Enum, float) override {return true;}
    void onEnter(Scene::Enum) override {
        for (auto wall: walls) {
            RENDERING(wall)-> show = true;
        }
        for (auto s: yEnnemies) {
            RENDERING(s)-> show = true;
        }
        for (auto p: players) {
            RENDERING(p)->show = true;
        }
        for (auto o: objs) {
            RENDERING(o)->show = true;
        }
        for (auto s: bEnnemies) {
            RENDERING(s)->show = true;
        }
        RENDERING(background)->show = true;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    Scene::Enum update(float) override { 
        return Scene::UserInput;
    }

    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {}
    bool updatePreExit(Scene::Enum, float) override {return true;}
    void onExit(Scene::Enum) override {
        for (auto wall: walls) {
            RENDERING(wall)-> show = false;
        }
        for (auto s: yEnnemies) {
            RENDERING(s)-> show = false;
        }
        for (auto p: players) {
            RENDERING(p)->show =false;
        }
        for (auto o: objs) {
            RENDERING(o)->show = false;
        }
        for (auto s: bEnnemies) {
            RENDERING(s)->show = false;
        }
        RENDERING(background)->show = false;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateUserInputSceneHandler(PrototypeGame* game) {
        return new UserInputScene(game);
    }
}

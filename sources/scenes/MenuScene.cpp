/*
    This file is part of RecursiveRunner.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

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
#include "gen/Scenes.h"

#include "systems/ButtonSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "base/PlacementHelper.h"

#include "../HerdingDogGame.h"

#include "base/SceneState.h"

#include <cstdio>

class MenuScene : public SceneState<Scene::Enum> {
    HerdingDogGame* game;
    std::list<Entity> buttons;

    public:

    MenuScene(HerdingDogGame* game) : SceneState<Scene::Enum>("menu", SceneEntityMode::Fading, SceneEntityMode::Fading) {
        this->game = game;
    }

    virtual void setup(AssetAPI* asset) override {
        std::list<std::string> list = asset->listAssetContent(".lvl");
        list.sort();
        list.insert(list.begin(), "Level editor");


        float buttonSize = .3 * (PlacementHelper::ScreenSize.y) / list.size();
        float ypos = (PlacementHelper::ScreenSize.y * 0.5);
        for (auto l : list) {
            Entity e = theEntityManager.CreateEntityFromTemplate("menu/button");
            TEXT(e)->text = l.c_str();
            TRANSFORM(e)->position.y = ypos ;
            ypos -= buttonSize + 1;
            TRANSFORM(e)->size.y = buttonSize;
            buttons.push_back(e);
            this->batch.addEntity(e);
        }
    }

    Scene::Enum update(float) override {
        if (BUTTON(*buttons.begin())->clicked) {
            return Scene::Editor;
        }

        for (auto e : buttons) {
            if(BUTTON(e)->clicked) {
                game->level = strdup((TEXT(e)->text + ".lvl").c_str());
                return  Scene::GameStart;
            }
        }

        return  Scene::Menu;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(HerdingDogGame* game) {
        return new MenuScene(game);
    }
}

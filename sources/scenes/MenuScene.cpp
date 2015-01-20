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

#include "../SacHelloWorldGame.h"

#include "base/SceneState.h"

#include <cstdio>

class MenuScene : public SceneState<Scene::Enum> {
    SacHelloWorldGame* game;
    std::list<Entity>  buttons;

    public:

    MenuScene(SacHelloWorldGame* game) : SceneState<Scene::Enum>("menu", SceneEntityMode::Fading, SceneEntityMode::Fading) {
        this->game = game;
    }

    virtual void setup(AssetAPI* asset) {
        std::list<std::string>  list = asset->listAssetContent( ".lvl" );
        list.sort();

        int i = 0;
        float  buttonSize = 10.0 / list.size();
        for(auto l: list){
            Entity e = theEntityManager.CreateEntityFromTemplate("menu/menubutton");
            TEXT(e)->text = l.c_str();
            TRANSFORM(e)->position.y = 7 - i * ( buttonSize + 1 ); ++i;
            TRANSFORM(e)->size.y = buttonSize;
            buttons.push_back(e);
        }
    }

    void onEnter( Scene::Enum ) {
        for( auto e: buttons){
            RENDERING( e )->show =
                TEXT( e )->show =
                BUTTON( e )->enabled = true;
        }
    }

    Scene::Enum update(float) {
        for( auto e: buttons){
            if( BUTTON( e )->clicked )
            {
                game->level = strdup( (TEXT(e)->text + ".lvl").c_str() );

                return  Scene::GameStart;
            }
        }

        return  Scene::Menu;
    }

    void onExit( Scene::Enum ) {
        for( auto e: buttons){
            RENDERING( e )->show =
                TEXT( e )->show =
                BUTTON( e )->enabled = false;
        }
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateMenuSceneHandler(SacHelloWorldGame* game) {
        return new MenuScene(game);
    }
}

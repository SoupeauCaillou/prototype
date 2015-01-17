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

#pragma once
#include <base/Game.h>
#include <base/StateMachine.h>
class PrototypeGame;
namespace Scene {
        enum Enum : int {
                Editor,
                GameEnd,
                GameStart,
                InGame,
                Logo,
                Menu,
        };
        StateHandler<Scene::Enum>* CreateEditorSceneHandler(PrototypeGame* game);
        StateHandler<Scene::Enum>* CreateGameEndSceneHandler(PrototypeGame* game);
        StateHandler<Scene::Enum>* CreateGameStartSceneHandler(PrototypeGame* game);
        StateHandler<Scene::Enum>* CreateInGameSceneHandler(PrototypeGame* game);
        StateHandler<Scene::Enum>* CreateLogoSceneHandler(PrototypeGame* game);
        StateHandler<Scene::Enum>* CreateMenuSceneHandler(PrototypeGame* game);
}
inline void registerScenes(PrototypeGame * game, StateMachine<Scene::Enum> & machine) { machine.registerState(Scene::Editor, Scene::CreateEditorSceneHandler(game));
        machine.registerState(Scene::GameEnd, Scene::CreateGameEndSceneHandler(game));
        machine.registerState(Scene::GameStart, Scene::CreateGameStartSceneHandler(game));
        machine.registerState(Scene::InGame, Scene::CreateInGameSceneHandler(game));
        machine.registerState(Scene::Logo, Scene::CreateLogoSceneHandler(game));
        machine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(game));
}
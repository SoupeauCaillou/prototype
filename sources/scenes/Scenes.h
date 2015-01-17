/*
    This file is part of SacHelloWorld.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    SacHelloWorld is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    SacHelloWorld is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SacHelloWorld.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <base/Game.h>
#include <base/StateMachine.h>
class SacHelloWorldGame;
namespace Scene {
    enum Enum : int {
        Editor,
        Game,
    };
    StateHandler<Scene::Enum>* CreateEditorSceneHandler(SacHelloWorldGame* game);
    StateHandler<Scene::Enum>* CreateGameSceneHandler(SacHelloWorldGame* game);
}
inline void registerScenes(SacHelloWorldGame * game, StateMachine<Scene::Enum> & machine) {    machine.registerState(Scene::Editor, Scene::CreateEditorSceneHandler(game));
    machine.registerState(Scene::Game, Scene::CreateGameSceneHandler(game));
}
/*
    This file is part of MyTest.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    MyTest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    MyTest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MyTest.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <base/Game.h>
#include <base/StateMachine.h>
class MyTestGame;
namespace Scene {
	enum Enum : int {
		BeginLoop,
		CreateLevel,
		EndLoop,
		Game,
		Menu,
		Objective,
		Victory,
	};
	StateHandler<Scene::Enum>* CreateBeginLoopSceneHandler(MyTestGame* game);
	StateHandler<Scene::Enum>* CreateCreateLevelSceneHandler(MyTestGame* game);
	StateHandler<Scene::Enum>* CreateEndLoopSceneHandler(MyTestGame* game);
	StateHandler<Scene::Enum>* CreateGameSceneHandler(MyTestGame* game);
	StateHandler<Scene::Enum>* CreateMenuSceneHandler(MyTestGame* game);
	StateHandler<Scene::Enum>* CreateObjectiveSceneHandler(MyTestGame* game);
	StateHandler<Scene::Enum>* CreateVictorySceneHandler(MyTestGame* game);
}
inline void registerScenes(MyTestGame * game, StateMachine<Scene::Enum> & machine) {	machine.registerState(Scene::BeginLoop, Scene::CreateBeginLoopSceneHandler(game));
	machine.registerState(Scene::CreateLevel, Scene::CreateCreateLevelSceneHandler(game));
	machine.registerState(Scene::EndLoop, Scene::CreateEndLoopSceneHandler(game));
	machine.registerState(Scene::Game, Scene::CreateGameSceneHandler(game));
	machine.registerState(Scene::Menu, Scene::CreateMenuSceneHandler(game));
	machine.registerState(Scene::Objective, Scene::CreateObjectiveSceneHandler(game));
	machine.registerState(Scene::Victory, Scene::CreateVictorySceneHandler(game));
}
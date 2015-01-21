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

#include "systems/GridSystem.h"
#include "systems/TextSystem.h"

#include "../HerdingDogGame.h"
#include "base/TouchInputManager.h"
#include "base/SceneState.h"


class LoseScene : public SceneState<Scene::Enum> {
    HerdingDogGame* game;

    public:

    LoseScene(HerdingDogGame* game) : SceneState<Scene::Enum>("lose", SceneEntityMode::Fading, SceneEntityMode::Fading) {
        this->game = game;
    }

    Scene::Enum update(float) override {
        if (theTouchInputManager.hasClicked()) {
            theGridSystem.deleteAllEntities();
            return Scene::Menu;
        }
        return Scene::Lose;
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateLoseSceneHandler(HerdingDogGame* game) {
        return new LoseScene(game);
    }
}

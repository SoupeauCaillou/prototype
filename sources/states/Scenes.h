/*
    This file is part of Bzzz.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Bzzz is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Bzzz is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bzzz.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

class BzzzGame;
template <class T> class StateHandler;

namespace Scene {
   enum Enum {
      Logo,
      Menu,
      GameStart,
      InGame,
      GameEnd,
      Count,
   };

#define DECLARE_SCENE_HANDLER_FACTORY(name) \
  StateHandler<Scene::Enum>* Create##name##SceneHandler(BzzzGame* game);

  DECLARE_SCENE_HANDLER_FACTORY(Logo)
  DECLARE_SCENE_HANDLER_FACTORY(Menu)
  DECLARE_SCENE_HANDLER_FACTORY(GameStart)
  DECLARE_SCENE_HANDLER_FACTORY(InGame)
  DECLARE_SCENE_HANDLER_FACTORY(GameEnd)
}

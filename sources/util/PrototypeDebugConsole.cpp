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


#if SAC_INGAME_EDITORS && SAC_DEBUG

#include "PrototypeDebugConsole.h"

#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"

#include "base/Log.h"
#include "base/EntityManager.h"

#include "PrototypeGame.h"

PrototypeGame* PrototypeDebugConsole::game = 0;

void PrototypeDebugConsole::init(PrototypeGame* g) {
    game = g;

    DebugConsole::RegisterMethod("Move all sheep to final zone", moveAllSheepToFinalZone);
    static std::string saveName = "/tmp/level_sheep.ini";
    DebugConsole::RegisterMethod("Save current level", saveLevel, "Save path",
        TW_TYPE_STDSTRING, &saveName);

    static std::string loadName = "/tmp/level_sheep.ini";
    DebugConsole::RegisterMethod("Load given level", loadLevel, "Load path",
        TW_TYPE_STDSTRING, &loadName);
}

void PrototypeDebugConsole::moveAllSheepToFinalZone(void*) {
    if (game->levelLoader.zones.size() == 0)
        return;

    Entity zone = game->levelLoader.zones[0];
    for (auto s : game->levelLoader.sheep) {
        TRANSFORM(s)->position = TRANSFORM(zone)->position;
    }
}

void PrototypeDebugConsole::saveLevel(void* arg) {
    std::string file = *((std::string*)arg);
    LOGI("Saved current level to file '" << file << "'");
    game->levelLoader.save(file);
}

void PrototypeDebugConsole::loadLevel(void* arg) {
    std::string file = *((std::string*)arg);
    LOGI("Load file '" << file << "'");
    FileBuffer fb = game->gameThreadContext->assetAPI->loadFile(file);
    game->levelLoader.load(fb);
}
#endif

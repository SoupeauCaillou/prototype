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

#include "PrototypeGame.h"
#include "app/AppSetup.h"
#include "PrototypeGitVersion.h"

int main(int argc, char** argv) {
    std::string title = "Raoul the farmer";
    #if SAC_DEBUG
        title = title + " - " + TAG_NAME + " - " + VERSION_NAME;
    #endif

    if (initGame(title, glm::ivec2(900, 562))) {
        LOGE("Failed to initialize");
        return 1;
    }
    return launchGame(new PrototypeGame(argc, argv), argc, argv);
}

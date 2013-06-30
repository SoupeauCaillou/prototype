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

#include "base/Entity.h"

#include "api/AssetAPI.h"

#include <string>
#include <vector>

class LevelLoader {
    public:
        static void SaveInFile(const std::string & filename, const std::vector<Entity> & spotList,
            const std::vector<std::pair<Entity, Entity>> & wallList, const float objDistance);

        static bool LoadFromFile(const std::string& ctx, const FileBuffer& fb);
};

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

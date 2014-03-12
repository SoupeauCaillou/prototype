#pragma once

#include <string>

#include <api/AssetAPI.h>

class LevelLoader {
    public:
        static void load(AssetAPI* assetAPI, const std::string & levelName);
};

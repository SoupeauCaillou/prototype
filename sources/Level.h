#pragma once

#include "api/AssetAPI.h"

class HexSpatialGrid;

class Level {
    public:
        static HexSpatialGrid* load(const FileBuffer& fb);
};

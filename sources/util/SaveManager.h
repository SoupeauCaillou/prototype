#pragma once

#include <string>
#include <vector>

#include <api/AssetAPI.h>
#include <base/Entity.h>

class SaveManager {
    public:
    	void init(AssetAPI* assetAPI);
        void load();        
        void save();

    private:
    	AssetAPI* assetAPI;
};

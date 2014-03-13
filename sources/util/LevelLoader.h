#pragma once

#include <string>
#include <vector>

#include <api/AssetAPI.h>
#include <base/Entity.h>

class LevelLoader {
    public:
    	void init(AssetAPI* assetAPI);
        void load(FileBuffer & fb);
        void save(const std::string & path);

        std::vector<Entity> sheep;
        std::vector<Entity> walls;
        std::vector<Entity> bushes;
        std::vector<Entity> zones;
        Entity background;
        
        int objectiveArrived;
        int objectiveSurvived;
        float objectiveTimeLimit;

	private:
		AssetAPI* assetAPI;

};

#pragma once

#include <string>
#include <vector>

#include <api/AssetAPI.h>
#include <base/Entity.h>

class LevelLoader {
    public:
    	void init(AssetAPI* assetAPI);
        void load(const std::string & levelName);


        std::vector<Entity> sheep;
        std::vector<Entity> walls;
        std::vector<Entity> bushes;
        Entity arrivalZone;
        int objectiveArrived;
        int objectiveSurvived;
        float objectiveTimeLimit;

	private:
		AssetAPI* assetAPI;

};

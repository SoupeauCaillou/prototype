#pragma once

#include <map>
#include <string>

#include <api/AssetAPI.h>
#include <base/Entity.h>

class PrototypeGame;

class SaveManager {
    public:
    	void init(PrototypeGame* prototypeGame);
        void load();        
        void save();

        std::string getValue(const std::string & key) const { 
            if (hasValue(key)) 
                return kv.find(key)->second; 
            else 
                return ""; 
        }
        bool hasValue(const std::string & key) const { return kv.find(key) != kv.end(); }
        void setValue(const std::string & key, const std::string & value) { kv[key] = value; }
    private:
        PrototypeGame* game;

        //do not use the map direcly (less error prone)
        std::map<std::string, std::string> kv;
};

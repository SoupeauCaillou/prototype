#pragma once

#include "systems/System.h"

struct LevelComponent {
    LevelComponent() : asciiMap("") {}

    std::string asciiMap;
};

#define theLevelSystem LevelSystem::GetInstance()
#define LEVEL(e) theLevelSystem.Get(e)

UPDATABLE_SYSTEM(Level)
    public:
        static void SaveInFile(const std::string & filename, const std::list<Entity> & wallList,  const std::list<Entity> & spotList);
        static void LoadFromFile(const std::string & filename);
};

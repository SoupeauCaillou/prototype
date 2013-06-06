#pragma once

#include "systems/System.h"

struct SpotComponent {
    SpotComponent() : dragStarted(false) {}

    bool dragStarted;

    //for each wall highlighted, get the first and last highlighted points
    std::list<std::pair<glm::vec2, glm::vec2>> highlightedEdges;
};

#define theSpotSystem SpotSystem::GetInstance()
#define SPOT(e) theSpotSystem.Get(e)

UPDATABLE_SYSTEM(Spot)
    public:
        void CleanEntities();
};

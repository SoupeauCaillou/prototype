#pragma once

#include "base/Entity.h"

#include <list>

class Grid {
    public:
        static void CreateGrid();

        static void EnableGrid();

        static void DisableGrid();

    private:
        static Grid & Instance();

        std::list<Entity> _gridEntities;
        std::list<Entity> _gridTextEntities;
};

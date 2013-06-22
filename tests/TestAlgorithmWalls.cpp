#include <UnitTest++.h>

#include "systems/SpotSystem.h"
#include "systems/BlockSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "base/PlacementHelper.h"
#include "util/DrawSomething.h"

#include <glm/gtx/vector_angle.hpp>
#include <fstream>

void AddSpot(const std::string & name, const glm::vec2 & pos) {
    Entity e = theEntityManager.CreateEntity(name);
    ADD_COMPONENT(e, Transformation);
    TRANSFORM(e)->position = pos;
    ADD_COMPONENT(e, Spot);
}

void AddWall(const std::string & name, const glm::vec2 & firstPoint, const glm::vec2 & secondPoint, bool isDoubleFace) {
    Entity e = theEntityManager.CreateEntity(name);
    ADD_COMPONENT(e, Transformation);
    ADD_COMPONENT(e, Block);

    TRANSFORM(e)->position = (secondPoint + firstPoint) / 2.f;
    TRANSFORM(e)->size.x = glm::length(secondPoint - firstPoint);
    TRANSFORM(e)->size.y = 0.1;
    TRANSFORM(e)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize( secondPoint - firstPoint));

    BLOCK(e)->isDoubleFace = isDoubleFace;
}

static void Init(std::stringstream & ss) {
    auto windowW = 900, windowH = 625;
    if (windowW < windowH) {
        PlacementHelper::ScreenHeight = 10;
        PlacementHelper::ScreenWidth = PlacementHelper::ScreenHeight * windowW / (float)windowH;
    } else {
        PlacementHelper::ScreenWidth = 20;
        PlacementHelper::ScreenHeight = PlacementHelper::ScreenWidth * windowH / (float)windowW;
    }

    PlacementHelper::WindowWidth = windowW;
    PlacementHelper::WindowHeight = windowH;


    SpotSystem::CreateInstance();
    BlockSystem::CreateInstance();
    RenderingSystem::CreateInstance(); //needed because we do some stuff with drawSomething class..
    TransformationSystem::CreateInstance();

    theSpotSystem.outputStream = ss.rdbuf();
}

static void CheckAndQuit(std::stringstream & ss, const std::vector<std::string> & expected) {
    //go back to the start of file, and compare result line by line
    ss.seekp(0);
    std::string line;
    for (unsigned i = 0; i < expected.size(); ++i) {
        CHECK(ss.good());
        getline(ss, line);
        CHECK_EQUAL(expected[i], line);
    }
    getline(ss, line); //remove the last empty line

    if (ss.good()) {
        do {
            std::cout << "Error! Line not treated: " << line << std::endl;
            getline(ss, line);
        } while (ss.good());
        CHECK(false);
    }

    Draw::Clear();

    SpotSystem::DestroyInstance();
    BlockSystem::DestroyInstance();
    RenderingSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
}

TEST(CheckWhenEmpty)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::POINTS_ORDER | SpotSystem::ACTIVE_WALL;

    //create the map
    AddSpot("spot1", glm::vec2(0.));

    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "Spot spot1",
        "wall middle left (first point)",
        "Active wall is -10.0, -6.9 <-> -10.0, 0.0",
        "wall top left",
        "Active wall is -10.0, 0.0 <-> -10.0, 6.9",
        "top right",
        "Active wall is -10.0, 6.9 <-> 10.0, 6.9",
        "wall bottom right",
        "Active wall is 10.0, 6.9 <-> 10.0, -6.9",
        "wall bottom left",
        "Active wall is 10.0, -6.9 <-> -10.0, -6.9",
        "Active wall is -10.0, -6.9 <-> -10.0, 0.0",
    };

    CheckAndQuit(ss, expected);
}

TEST(Check1SpotAndATriangle)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::POINTS_ORDER | SpotSystem::CALCULATION_ALGO;

    //create the map
    AddSpot("spot1", glm::vec2(-5, 3.4));

    AddWall("block1", glm::vec2(-4, -0.4), glm::vec2(0, -3), false);
    AddWall("block2", glm::vec2(0, -3), glm::vec2(2.7, 1.2), false);
    AddWall("block3", glm::vec2(2.7, 1.2), glm::vec2(-4, -.4), false);

    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "Spot spot1",
        "wall middle left (first point)",
        "wall top left",
        "new zone: 3.5",
        "top right",
        "new zone: 20.0",
        "block2- second point",
        "new zone: 7.8",
        "wall bottom right",
        "new zone: 3.4",
        "block1- second point",
        "new zone: 1.7",
        "block1- first point",
        "new zone: 1.7",
        "wall bottom left",
        "new zone: 8.9",
        "new zone: 10.3",
        "totalHighlightedDistance2Objective=84.4 and totalHighlightedDistance2Done=57.5",
        "highlighted: -10.0, 6.9 <-> 10.0, 6.9",
        "highlighted: 10.0, 6.9 <-> 10.0, -0.9",
        "highlighted: 2.7, 1.2 <-> -4.0, -0.4",
        "highlighted: -4.0, -0.4 <-> -1.7, -6.9",
        "highlighted: -1.7, -6.9 <-> -10.0, -6.9",
        "highlighted: -10.0, -6.9 <-> -10.0, 6.9",

    };

    CheckAndQuit(ss, expected);
}
#if 0
TEST(Check2SpotsAndATriangle)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::POINTS_ORDER | SpotSystem::CALCULATION_ALGO;

    //create the map
    AddSpot("spot1", glm::vec2(-5, 3.4));
    AddSpot("spot2", glm::vec2(4.5, -2.2));

    AddWall("block1", glm::vec2(-4, -0.4), glm::vec2(0, -3), false);
    AddWall("block2", glm::vec2(0, -3), glm::vec2(2.7, 1.2), false);
    AddWall("block3", glm::vec2(2.7, 1.2), glm::vec2(-4, -.4), false);

    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "Spot spot1",
        "wall middle left (first point)",
        "wall top left",
        "new zone: 3.5",
        "top right",
        "new zone: 20.0",
        "block2- second point",
        "new zone: 7.8",
        "wall bottom right",
        "new zone: 3.4",
        "block1- second point",
        "new zone: 1.7",
        "block1- first point",
        "new zone: 1.7",
        "wall bottom left",
        "new zone: 8.9",
        "new zone: 10.3",
        "Spot spot2",
        "wall middle left (first point)",
        "block1- first point",
        "new zone: 0.9",
        "wall top left",
        "new zone: 1.2",
        "block2- second point",
        "new zone: 1.9",
        "top right",
        "new zone: 10.3",
        "wall bottom right",
        "new zone: 13.9",
        "wall bottom left",
        "new zone: 20.0",
        "block1- second point",
        "new zone: 2.2",
        "new zone: 1.0",
        "totalHighlightedDistance2Objective=84.4 and totalHighlightedDistance2Done=88.5",
        "highlighted: -10.0, 6.9 <-> 10.0, 6.9",
        "highlighted: 10.0, 6.9 <-> 10.0, -0.9",
        "highlighted: 2.7, 1.2 <-> -4.0, -0.4",
        "highlighted: -4.0, -0.4 <-> -1.7, -6.9",
        "highlighted: -1.7, -6.9 <-> -10.0, -6.9",
        "highlighted: -10.0, -6.9 <-> -10.0, 6.9",
        "highlighted: 0.5, -2.2 <-> 1.0, -1.5",
        "highlighted: 1.0, -1.5 <-> 1.7, -0.4",
        "highlighted: 1.7, -0.4 <-> 2.7, 1.2",
        "highlighted: -0.3, 6.9 <-> 10.0, 6.9",
        "highlighted: 10.0, 6.9 <-> 10.0, -6.9",
        "highlighted: 10.0, -6.9 <-> -10.0, -6.9",
        "highlighted: -10.0, -6.9 <-> -10.0, -4.8",
        "highlighted: 0.0, -3.0 <-> 0.5, -2.2",

    };

    CheckAndQuit(ss, expected);
}
#endif

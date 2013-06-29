#if SAC_DEBUG
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
    auto windowSize = glm::vec2(900, 625);
    if (windowSize.x < windowSize.y) {
        PlacementHelper::ScreenSize.x = 10;
        PlacementHelper::ScreenSize.y = PlacementHelper::ScreenSize.y * windowSize.x / (float)windowSize.y;
    } else {
        PlacementHelper::ScreenSize.x = 20;
        PlacementHelper::ScreenSize.y = PlacementHelper::ScreenSize.x * windowSize.y / (float)windowSize.x;
    }

    PlacementHelper::WindowSize = windowSize;


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

TEST(CheckIntersectionSplitter2WallsAtOrigin)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::INTERSECTIONS_SPLIT;

    //create the map
    AddSpot("spot1", glm::vec2(2.));


    AddWall("block1", glm::vec2(-1, 0), glm::vec2(1, 0), false);
    AddWall("block2", glm::vec2(0, -1), glm::vec2(0, 1), false);

    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "name='block1- first point': position='-1.0, 0.0' nextEdge1='1.0, 0.0, ",
        "name='block1- second point': position='1.0, 0.0' nextEdge1='-1.0, 0.0, ",
        "name='block2- first point': position='0.0, -1.0' nextEdge1='-0.0, 1.0, ",
        "name='block2- second point': position='-0.0, 1.0' nextEdge1='0.0, -1.0, ",
        "name='wall top left': position='-10.0, 6.9' nextEdge1='10.0, 6.9, ",
        "name='top right': position='10.0, 6.9' nextEdge1='10.0, -6.9, ",
        "name='wall bottom right': position='10.0, -6.9' nextEdge1='-10.0, -6.9, ",
        "name='wall bottom left': position='-10.0, -6.9' nextEdge1='-10.0, 1000.0, ",
        "before splitIntersectionWalls",
        "after splitIntersectionWalls",
        "name='block1- first point': position='-1.0, 0.0' nextEdge1='-0.0, 0.0, ",
        "name='block1- second point': position='1.0, 0.0' nextEdge1='0.0, 0.0, ",
        "name='block2- first point': position='0.0, -1.0' nextEdge1='-0.0, 0.0, ",
        "name='block2- second point': position='-0.0, 1.0' nextEdge1='0.0, 0.0, ",
        "name='wall top left': position='-10.0, 6.9' nextEdge1='10.0, 6.9, ",
        "name='top right': position='10.0, 6.9' nextEdge1='10.0, -6.9, ",
        "name='wall bottom right': position='10.0, -6.9' nextEdge1='-10.0, -6.9, ",
        "name='wall bottom left': position='-10.0, -6.9' nextEdge1='-10.0, 1000.0, ",
        "name='intersection point': position='-0.0, 0.0' nextEdge1='1.0, 0.0, ' nextEdge2='-0.0, 1.0, ' nextEdge3='-1.0, 0.0, ' nextEdge4='0.0, -1.0, ",
    };

    CheckAndQuit(ss, expected);
}

TEST(CheckIntersectionSplitterWithExternalWall)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::INTERSECTIONS_SPLIT;

    //create the map
    AddSpot("spot1", glm::vec2(2.));


    AddWall("block1", glm::vec2(0, 0), glm::vec2(10, 0), false);

    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "name='block1- first point': position='0.0, 0.0' nextEdge1='10.0, 0.0, ",
        "name='block1- second point': position='10.0, 0.0' nextEdge1='0.0, 0.0, ",
        "name='wall top left': position='-10.0, 6.9' nextEdge1='10.0, 6.9, ",
        "name='top right': position='10.0, 6.9' nextEdge1='10.0, -6.9, ",
        "name='wall bottom right': position='10.0, -6.9' nextEdge1='-10.0, -6.9, ",
        "name='wall bottom left': position='-10.0, -6.9' nextEdge1='-10.0, 1000.0, ",
        "before splitIntersectionWalls",
        "after splitIntersectionWalls",
        "name='block1- first point': position='0.0, 0.0' nextEdge1='10.0, 0.0, ",
        "name='block1- second point': position='10.0, 0.0' nextEdge1='0.0, 0.0, ",
        "name='wall top left': position='-10.0, 6.9' nextEdge1='10.0, 6.9, ",
        "name='top right': position='10.0, 6.9' nextEdge1='10.0, -6.9, ",
        "name='wall bottom right': position='10.0, -6.9' nextEdge1='-10.0, -6.9, ",
        "name='wall bottom left': position='-10.0, -6.9' nextEdge1='-10.0, 1000.0, ",

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
        "top right",
        "block2- second point",
        "wall bottom right",
        "block1- second point",
        "block1- first point",
        "wall bottom left",
        "totalHighlightedDistance2Objective=84.4 and totalHighlightedDistance2Done=56.3",
        "highlighted: -10.0, 6.9 <-> 10.0, 6.9",
        "highlighted: 10.0, 6.9 <-> 10.0, -0.9",
        "highlighted: 2.7, 1.2 <-> -4.0, -0.4",
        "highlighted: -2.3, -6.9 <-> -10.0, -6.9",
        "highlighted: -10.0, -6.9 <-> -10.0, 6.9",

    };

    CheckAndQuit(ss, expected);
}

TEST(Check2SpotsAndATriangle)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::CALCULATION_ALGO;

    //create the map
    AddSpot("spot1", glm::vec2(-5, 3.4));
    AddSpot("spot2", glm::vec2(6.7778, 0.6556));

    AddWall("block1", glm::vec2(-4, -0.4), glm::vec2(0, -3), false);
    AddWall("block2", glm::vec2(0, -3), glm::vec2(2.7, 1.2), false);
    AddWall("block3", glm::vec2(2.7, 1.2), glm::vec2(-4, -.4), false);

    //ensure that multiple updates does not broke the system
    theSpotSystem.Update(1);
    theSpotSystem.Update(1);

    ss.str("");
    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "totalHighlightedDistance2Objective=84.4 and totalHighlightedDistance2Done=79.7",
        "highlighted: 2.7, 1.2 <-> -4.0, -0.4",
        "highlighted: -10.0, -6.9 <-> -10.0, 6.9",
        "highlighted: -10.0, 6.9 <-> 10.0, 6.9",
        "highlighted: 10.0, 6.9 <-> 10.0, -6.9",
        "highlighted: 10.0, -6.9 <-> -10.0, -6.9",
        "highlighted: 0.0, -3.0 <-> 2.7, 1.2",

    };

    CheckAndQuit(ss, expected);
}


TEST(Check2SpotsAndATriangleAgain)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::CALCULATION_ALGO;

    //create the map
    AddSpot("spot1", glm::vec2(-5, 3.4));
    AddSpot("spot2", glm::vec2(2.0667, 3.8778));

    AddWall("block1", glm::vec2(-4, -0.4), glm::vec2(0, -3), false);
    AddWall("block2", glm::vec2(0, -3), glm::vec2(2.7, 1.2), false);
    AddWall("block3", glm::vec2(2.7, 1.2), glm::vec2(-4, -.4), false);

    //ensure that multiple updates does not broke the system
    theSpotSystem.Update(1);
    theSpotSystem.Update(1);

    ss.str("");
    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "totalHighlightedDistance2Objective=84.4 and totalHighlightedDistance2Done=67.8",
        "highlighted: -2.3, -6.9 <-> -10.0, -6.9",
        "highlighted: -10.0, 6.9 <-> 10.0, 6.9",
        "highlighted: 10.0, 6.9 <-> 10.0, -6.9",
        "highlighted: 10.0, -6.9 <-> 4.6, -6.9",
        "highlighted: 2.7, 1.2 <-> -4.0, -0.4",
        "highlighted: -10.0, -6.9 <-> -10.0, 6.9",

    };

    CheckAndQuit(ss, expected);
}

TEST(Check2SpotsAndATriangleAgainAgain)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::CALCULATION_ALGO;

    //create the map
    AddSpot("spot1", glm::vec2(-5, 3.4));
    AddSpot("spot2", glm::vec2(6.1111, 2.5000));

    AddWall("block1", glm::vec2(-4, -0.4), glm::vec2(0, -3), false);
    AddWall("block2", glm::vec2(0, -3), glm::vec2(2.7, 1.2), false);
    AddWall("block3", glm::vec2(2.7, 1.2), glm::vec2(-4, -.4), false);

    //ensure that multiple updates does not broke the system
    theSpotSystem.Update(1);
    theSpotSystem.Update(1);

    ss.str("");
    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "totalHighlightedDistance2Objective=84.4 and totalHighlightedDistance2Done=79.7",
        "highlighted: -10.0, 6.9 <-> 10.0, 6.9",
        "highlighted: 10.0, 6.9 <-> 10.0, -6.9",
        "highlighted: 10.0, -6.9 <-> -10.0, -6.9",
        "highlighted: 0.0, -3.0 <-> 2.7, 1.2",
        "highlighted: 2.7, 1.2 <-> -4.0, -0.4",
        "highlighted: -10.0, -6.9 <-> -10.0, 6.9",

    };

    CheckAndQuit(ss, expected);
}

#endif

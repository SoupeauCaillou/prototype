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
    theSpotSystem.useOptimization = false;
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
    theSpotSystem.PrepareAlgorithm();
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
#if 0
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
    theSpotSystem.PrepareAlgorithm();
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "name='block1- first point': position='-1.0, 0.0' nextEdge1='1.0, 0.0, ",
        "name='block1- second point': position='1.0, 0.0' nextEdge1='-1.0, 0.0, ",
        "name='block2- first point': position='0.0, -1.0' nextEdge1='-0.0, 1.0, ",
        "name='block2- second point': position='-0.0, 1.0' nextEdge1='0.0, -1.0, ",
        "name='wall top left': position='-10.0, 6.9' nextEdge1='10.0, 6.9, ",
        "name='top right': position='10.0, 6.9' nextEdge1='10.0, -6.9, ",
        "name='wall bottom right': position='10.0, -6.9' nextEdge1='-10.0, -6.9, ",
        "name='wall bottom left': position='-10.0, -6.9' nextEdge1='-10.0, 6.9, ",
        "before splitIntersectionWalls",
        "after splitIntersectionWalls",
        "name='block1- first point': position='-1.0, 0.0' nextEdge1='-0.0, 0.0, ",
        "name='block1- second point': position='1.0, 0.0' nextEdge1='0.0, 0.0, ",
        "name='block2- first point': position='0.0, -1.0' nextEdge1='-0.0, 0.0, ",
        "name='block2- second point': position='-0.0, 1.0' nextEdge1='0.0, 0.0, ",
        "name='wall top left': position='-10.0, 6.9' nextEdge1='10.0, 6.9, ",
        "name='top right': position='10.0, 6.9' nextEdge1='10.0, -6.9, ",
        "name='wall bottom right': position='10.0, -6.9' nextEdge1='-10.0, -6.9, ",
        "name='wall bottom left': position='-10.0, -6.9' nextEdge1='-10.0, 6.9, ",
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
    theSpotSystem.PrepareAlgorithm();
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "name='block1- first point': position='0.0, 0.0' nextEdge1='10.0, 0.0, ",
        "name='block1- second point': position='10.0, 0.0' nextEdge1='0.0, 0.0, ",
        "name='wall top left': position='-10.0, 6.9' nextEdge1='10.0, 6.9, ",
        "name='top right': position='10.0, 6.9' nextEdge1='10.0, -6.9, ",
        "name='wall bottom right': position='10.0, -6.9' nextEdge1='-10.0, -6.9, ",
        "name='wall bottom left': position='-10.0, -6.9' nextEdge1='-10.0, 6.9, ",
        "before splitIntersectionWalls",
        "after splitIntersectionWalls",
        "name='block1- first point': position='0.0, 0.0' nextEdge1='10.0, 0.0, ",
        "name='block1- second point': position='10.0, 0.0' nextEdge1='0.0, 0.0, ",
        "name='wall top left': position='-10.0, 6.9' nextEdge1='10.0, 6.9, ",
        "name='top right': position='10.0, 6.9' nextEdge1='10.0, -6.9, ",
        "name='wall bottom right': position='10.0, -6.9' nextEdge1='-10.0, -6.9, ",
        "name='wall bottom left': position='-10.0, -6.9' nextEdge1='-10.0, 6.9, ",

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
    theSpotSystem.PrepareAlgorithm();
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
    theSpotSystem.PrepareAlgorithm();
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
    theSpotSystem.PrepareAlgorithm();
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
    theSpotSystem.PrepareAlgorithm();
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

TEST(CheckBigMapDistance)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::HIGHLIGHTED_WALLS_BEFORE_MERGE;

    //create the map
    AddSpot("spot1", glm::vec2(-6.9333, 2.1889));
    AddSpot("spot2", glm::vec2(5.4667, 5.9000));
    AddSpot("spot3", glm::vec2(8.8667, -5.3889));
    AddSpot("spot4", glm::vec2(-5.3111, -4.1000));

    AddWall("block1", glm::vec2(-10, 1), glm::vec2(-8, 1), false);
    AddWall("block2", glm::vec2(-8, 1), glm::vec2(-8, -1), false);
    AddWall("block3", glm::vec2(-10, -1), glm::vec2(-8, -1), false);
    AddWall("block4", glm::vec2(-10, -5), glm::vec2(-9, -5), false);
    AddWall("block5", glm::vec2(-9, -5), glm::vec2(-9, -7), false);
    AddWall("block6", glm::vec2(-1, -7), glm::vec2(-1, -2), false);
    AddWall("block7", glm::vec2(-2, -2), glm::vec2(-1, -2), false);
    AddWall("block8", glm::vec2(-2, -2), glm::vec2(-2, -1), false);
    AddWall("block9", glm::vec2(-2, -1), glm::vec2(-1, -1), false);
    AddWall("block10", glm::vec2(-1, -1), glm::vec2(-1, 0), false);
    AddWall("block11", glm::vec2(-1, 0), glm::vec2(1, 0), false);
    AddWall("block12", glm::vec2(1, -1), glm::vec2(1, 0), false);
    AddWall("block13", glm::vec2(2, -1), glm::vec2(1, -1), false);
    AddWall("block14", glm::vec2(2, -2), glm::vec2(2, -1), false);
    AddWall("block15", glm::vec2(2, -2), glm::vec2(1, -2), false);
    AddWall("block16", glm::vec2(1, -7), glm::vec2(1, -2), false);
    AddWall("block17", glm::vec2(10, -2), glm::vec2(8, -2), false);
    AddWall("block18", glm::vec2(8, -2), glm::vec2(8, -3), false);
    AddWall("block19", glm::vec2(8, -3), glm::vec2(7, -3), false);
    AddWall("block20", glm::vec2(7, -3), glm::vec2(7, -1), false);
    AddWall("block21", glm::vec2(7, -1), glm::vec2(10, -1), false);
    AddWall("block22", glm::vec2(10, 4), glm::vec2(8, 4), false);
    AddWall("block23", glm::vec2(8, 4), glm::vec2(8, 5), false);
    AddWall("block24", glm::vec2(8, 5), glm::vec2(10, 5), false);
    AddWall("block25", glm::vec2(2, 5), glm::vec2(2, 7), false);
    AddWall("block26", glm::vec2(-3, 7), glm::vec2(-3, 4), false);
    AddWall("block27", glm::vec2(-3, 4), glm::vec2(5, 4), false);
    AddWall("block28", glm::vec2(5, 4), glm::vec2(5, 5), false);
    AddWall("block29", glm::vec2(5, 5), glm::vec2(2, 5), false);

    //ensure that multiple updates does not broke the system
    theSpotSystem.PrepareAlgorithm();
    theSpotSystem.Update(1);
    theSpotSystem.Update(1);

    ss.str("");
    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "Spot at position -6.9, 2.2",
        "-10.0, 2.2 <-> -10.0, 6.9",
        "-10.0, 6.9 <-> -3.0, 6.9",
        "-3.0, 6.9 <-> -3.0, 6.9",
        "-3.0, 6.9 <-> -3.0, 4.3",
        "-3.0, 4.3 <-> -3.0, 4.3",
        "-3.0, 4.3 <-> -3.0, 4.0",
        "-3.0, 4.0 <-> -1.2, 4.0",
        "-1.2, 4.0 <-> -0.5, 4.0",
        "-0.5, 4.0 <-> 0.8, 4.0",
        "0.8, 4.0 <-> 2.7, 4.0",
        "2.7, 4.0 <-> 4.0, 4.0",
        "4.0, 4.0 <-> 5.0, 4.0",
        "8.0, 4.5 <-> 8.0, 4.0",
        "8.0, 4.0 <-> 10.0, 4.0",
        "10.0, 4.0 <-> 10.0, -1.0",
        "10.0, -1.0 <-> 7.0, -1.0",
        "7.0, -1.0 <-> 7.0, -1.3",
        "7.0, -1.3 <-> 7.0, -1.7",
        "1.0, 0.0 <-> 0.9, 0.0",
        "0.9, 0.0 <-> -0.6, 0.0",
        "-0.6, 0.0 <-> -0.8, 0.0",
        "-0.8, 0.0 <-> -1.0, 0.0",
        "-1.0, 0.0 <-> -1.0, -0.0",
        "-1.0, -0.0 <-> -1.0, -0.2",
        "-1.0, -0.2 <-> -1.0, -0.6",
        "-1.0, -0.6 <-> -1.0, -0.9",
        "-1.0, -0.9 <-> -1.0, -1.0",
        "-1.0, -1.0 <-> -1.0, -1.0",
        "-1.0, -1.0 <-> -2.0, -1.0",
        "-2.0, -1.0 <-> -2.0, -1.3",
        "-2.0, -1.3 <-> -2.0, -2.0",
        "-1.0, -2.8 <-> -1.0, -4.6",
        "-1.0, -4.6 <-> -1.0, -4.7",
        "-1.0, -4.7 <-> -1.0, -6.9",
        "-1.0, -6.9 <-> -1.0, -6.9",
        "-1.0, -6.9 <-> -9.0, -6.9",
        "-9.0, -6.9 <-> -9.0, -6.9",
        "-9.0, -6.9 <-> -9.0, -5.0",
        "-9.0, -5.0 <-> -9.3, -5.0",
        "-8.0, -1.0 <-> -8.0, -1.0",
        "-8.0, -1.0 <-> -8.0, -0.3",
        "-8.0, -0.3 <-> -8.0, 1.0",
        "-8.0, 1.0 <-> -8.1, 1.0",
        "-8.1, 1.0 <-> -10.0, 1.0",
        "-10.0, 1.0 <-> -10.0, 2.2",
        "Total distance: 215.2",
        "Spot at position 5.5, 5.9",
        "2.0, 5.9 <-> 2.0, 6.1",
        "2.0, 6.1 <-> 2.0, 6.3",
        "2.0, 6.3 <-> 2.0, 6.4",
        "2.0, 6.4 <-> 2.0, 6.9",
        "2.0, 6.9 <-> 2.2, 6.9",
        "2.2, 6.9 <-> 10.0, 6.9",
        "10.0, 6.9 <-> 10.0, 5.0",
        "10.0, 5.0 <-> 8.0, 5.0",
        "8.0, 5.0 <-> 8.0, 4.8",
        "8.0, 4.8 <-> 8.0, 4.0",
        "10.0, 2.5 <-> 10.0, -1.0",
        "10.0, -1.0 <-> 9.4, -1.0",
        "9.4, -1.0 <-> 7.9, -1.0",
        "7.9, -1.0 <-> 7.7, -1.0",
        "7.7, -1.0 <-> 7.4, -1.0",
        "7.4, -1.0 <-> 7.0, -1.0",
        "7.0, -1.0 <-> 7.0, -3.0",
        "7.7, -6.9 <-> 2.3, -6.9",
        "5.0, 4.0 <-> 5.0, 4.6",
        "5.0, 4.6 <-> 5.0, 4.6",
        "5.0, 4.6 <-> 5.0, 4.8",
        "5.0, 4.8 <-> 5.0, 5.0",
        "5.0, 5.0 <-> 5.0, 5.0",
        "5.0, 5.0 <-> 5.0, 5.0",
        "5.0, 5.0 <-> 5.0, 5.0",
        "5.0, 5.0 <-> 5.0, 5.0",
        "5.0, 5.0 <-> 4.9, 5.0",
        "4.9, 5.0 <-> 4.8, 5.0",
        "4.8, 5.0 <-> 4.7, 5.0",
        "4.7, 5.0 <-> 4.6, 5.0",
        "4.6, 5.0 <-> 4.6, 5.0",
        "4.6, 5.0 <-> 4.5, 5.0",
        "4.5, 5.0 <-> 4.5, 5.0",
        "4.5, 5.0 <-> 4.5, 5.0",
        "4.5, 5.0 <-> 4.5, 5.0",
        "4.5, 5.0 <-> 4.4, 5.0",
        "4.4, 5.0 <-> 4.3, 5.0",
        "4.3, 5.0 <-> 4.2, 5.0",
        "4.2, 5.0 <-> 3.7, 5.0",
        "3.7, 5.0 <-> 3.4, 5.0",
        "3.4, 5.0 <-> 3.0, 5.0",
        "3.0, 5.0 <-> 2.6, 5.0",
        "2.6, 5.0 <-> 2.0, 5.0",
        "2.0, 5.0 <-> 2.0, 5.1",
        "2.0, 5.1 <-> 2.0, 5.9",
        "Total distance: 120.4",
        "Spot at position 8.9, -5.4",
        "1.0, -5.4 <-> 1.0, -5.2",
        "1.0, -5.2 <-> 1.0, -5.2",
        "1.0, -5.2 <-> 1.0, -3.6",
        "1.0, -3.6 <-> 1.0, -3.3",
        "1.0, -3.3 <-> 1.0, -2.9",
        "1.0, -2.9 <-> 1.0, -2.7",
        "1.0, -2.7 <-> 1.0, -2.7",
        "1.0, -2.7 <-> 1.0, -2.4",
        "1.0, -2.4 <-> 1.0, -2.2",
        "1.0, -2.2 <-> 1.0, -2.0",
        "1.0, -2.0 <-> 1.2, -2.0",
        "1.2, -2.0 <-> 2.0, -2.0",
        "2.0, -2.0 <-> 2.0, -1.6",
        "2.0, -1.6 <-> 2.0, -1.6",
        "2.0, -1.6 <-> 2.0, -1.0",
        "1.0, -0.4 <-> 1.0, -0.2",
        "1.0, -0.2 <-> 1.0, 0.0",
        "-9.1, 6.9 <-> -6.7, 6.9",
        "-3.0, 4.0 <-> -0.2, 4.0",
        "-0.2, 4.0 <-> -0.1, 4.0",
        "-0.1, 4.0 <-> 1.5, 4.0",
        "7.0, -3.0 <-> 7.3, -3.0",
        "7.3, -3.0 <-> 7.5, -3.0",
        "7.5, -3.0 <-> 7.5, -3.0",
        "7.5, -3.0 <-> 7.9, -3.0",
        "7.9, -3.0 <-> 7.9, -3.0",
        "7.9, -3.0 <-> 8.0, -3.0",
        "8.0, -3.0 <-> 8.0, -3.0",
        "8.0, -3.0 <-> 8.0, -2.0",
        "8.0, -2.0 <-> 8.6, -2.0",
        "8.6, -2.0 <-> 8.6, -2.0",
        "8.6, -2.0 <-> 9.2, -2.0",
        "9.2, -2.0 <-> 9.2, -2.0",
        "9.2, -2.0 <-> 9.3, -2.0",
        "9.3, -2.0 <-> 9.7, -2.0",
        "9.7, -2.0 <-> 10.0, -2.0",
        "10.0, -2.0 <-> 10.0, -6.9",
        "10.0, -6.9 <-> 1.3, -6.9",
        "1.3, -6.9 <-> 1.0, -6.9",
        "1.0, -6.9 <-> 1.0, -6.7",
        "1.0, -6.7 <-> 1.0, -6.6",
        "1.0, -6.6 <-> 1.0, -6.1",
        "1.0, -6.1 <-> 1.0, -6.1",
        "1.0, -6.1 <-> 1.0, -6.0",
        "1.0, -6.0 <-> 1.0, -5.4",
        "Total distance: 124.6",
        "Spot at position -5.3, -4.1",
        "-10.0, -4.1 <-> -10.0, -1.0",
        "-10.0, -1.0 <-> -8.2, -1.0",
        "-8.2, -1.0 <-> -8.0, -1.0",
        "-8.0, -1.0 <-> -8.0, 1.0",
        "-10.0, 4.8 <-> -10.0, 6.9",
        "-10.0, 6.9 <-> -3.0, 6.9",
        "-3.0, 6.9 <-> -3.0, 6.9",
        "-3.0, 6.9 <-> -3.0, 4.0",
        "-3.0, 4.0 <-> 0.0, 4.0",
        "0.0, 4.0 <-> 0.1, 4.0",
        "0.1, 4.0 <-> 1.2, 4.0",
        "1.2, 4.0 <-> 3.2, 4.0",
        "-1.0, 0.0 <-> -1.0, -0.1",
        "-2.0, -1.0 <-> -2.0, -1.2",
        "-2.0, -1.2 <-> -2.0, -1.5",
        "-2.0, -1.5 <-> -2.0, -1.7",
        "-2.0, -1.7 <-> -2.0, -1.7",
        "-2.0, -1.7 <-> -2.0, -1.8",
        "-2.0, -1.8 <-> -2.0, -1.9",
        "-2.0, -1.9 <-> -2.0, -2.0",
        "-2.0, -2.0 <-> -1.9, -2.0",
        "-1.9, -2.0 <-> -1.8, -2.0",
        "-1.8, -2.0 <-> -1.3, -2.0",
        "-1.3, -2.0 <-> -1.0, -2.0",
        "-1.0, -2.0 <-> -1.0, -2.0",
        "-1.0, -2.0 <-> -1.0, -2.3",
        "-1.0, -2.3 <-> -1.0, -2.7",
        "-1.0, -2.7 <-> -1.0, -2.9",
        "-1.0, -2.9 <-> -1.0, -3.0",
        "-1.0, -3.0 <-> -1.0, -3.2",
        "-1.0, -3.2 <-> -1.0, -3.4",
        "-1.0, -3.4 <-> -1.0, -3.5",
        "-1.0, -3.5 <-> -1.0, -3.7",
        "-1.0, -3.7 <-> -1.0, -3.7",
        "-1.0, -3.7 <-> -1.0, -4.9",
        "-1.0, -4.9 <-> -1.0, -6.0",
        "-1.0, -6.0 <-> -1.0, -6.1",
        "-1.0, -6.1 <-> -1.0, -6.9",
        "-1.0, -6.9 <-> -1.1, -6.9",
        "-1.1, -6.9 <-> -8.9, -6.9",
        "-8.9, -6.9 <-> -9.0, -6.9",
        "-9.0, -6.9 <-> -9.0, -6.3",
        "-9.0, -6.3 <-> -9.0, -5.0",
        "-9.0, -5.0 <-> -10.0, -5.0",
        "-10.0, -5.0 <-> -10.0, -4.1",
        "Total distance: 163.5",
    };

    CheckAndQuit(ss, expected);
}

TEST(CheckBigMapMergedWalls)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::CALCULATION_ALGO;

    //create the map
    // AddSpot("spot1", glm::vec2(-4.2889, 1.1222));
    AddSpot("spot2", glm::vec2(7.0667, 5.7222));
    // AddSpot("spot3", glm::vec2(6.0667, -5.7889));
    // AddSpot("spot4", glm::vec2(-6.2444, -3.3667));


    AddWall("block1", glm::vec2(-10, 1), glm::vec2(-8, 1), false);
    AddWall("block2", glm::vec2(-8, 1), glm::vec2(-8, -1), false);
    AddWall("block3", glm::vec2(-10, -1), glm::vec2(-8, -1), false);
    AddWall("block4", glm::vec2(-10, -5), glm::vec2(-9, -5), false);
    AddWall("block5", glm::vec2(-9, -5), glm::vec2(-9, -7), false);
    AddWall("block6", glm::vec2(-1, -7), glm::vec2(-1, -2), false);
    AddWall("block7", glm::vec2(-2, -2), glm::vec2(-1, -2), false);
    AddWall("block8", glm::vec2(-2, -2), glm::vec2(-2, -1), false);
    AddWall("block9", glm::vec2(-2, -1), glm::vec2(-1, -1), false);
    AddWall("block10", glm::vec2(-1, -1), glm::vec2(-1, 0), false);
    AddWall("block11", glm::vec2(-1, 0), glm::vec2(1, 0), false);
    AddWall("block12", glm::vec2(1, -1), glm::vec2(1, 0), false);
    AddWall("block13", glm::vec2(2, -1), glm::vec2(1, -1), false);
    AddWall("block14", glm::vec2(2, -2), glm::vec2(2, -1), false);
    AddWall("block15", glm::vec2(2, -2), glm::vec2(1, -2), false);
    AddWall("block16", glm::vec2(1, -7), glm::vec2(1, -2), false);
    AddWall("block17", glm::vec2(10, -2), glm::vec2(8, -2), false);
    AddWall("block18", glm::vec2(8, -2), glm::vec2(8, -3), false);
    AddWall("block19", glm::vec2(8, -3), glm::vec2(7, -3), false);
    AddWall("block20", glm::vec2(7, -3), glm::vec2(7, -1), false);
    AddWall("block21", glm::vec2(7, -1), glm::vec2(10, -1), false);
    AddWall("block22", glm::vec2(10, 4), glm::vec2(8, 4), false);
    AddWall("block23", glm::vec2(8, 4), glm::vec2(8, 5), false);
    AddWall("block24", glm::vec2(8, 5), glm::vec2(10, 5), false);
    AddWall("block25", glm::vec2(2, 5), glm::vec2(2, 7), false);
    AddWall("block26", glm::vec2(-3, 7), glm::vec2(-3, 4), false);
    AddWall("block27", glm::vec2(-3, 4), glm::vec2(5, 4), false);
    AddWall("block28", glm::vec2(5, 4), glm::vec2(5, 5), false);
    AddWall("block29", glm::vec2(5, 5), glm::vec2(2, 5), false);


    //ensure that multiple updates does not broke the system
    theSpotSystem.PrepareAlgorithm();
    theSpotSystem.Update(1);
    theSpotSystem.Update(1);

    ss.str("");
    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "totalHighlightedDistance2Objective=144.8 and totalHighlightedDistance2Done=36.3",
        "highlighted: 2.0, 6.9 <-> 10.0, 6.9",
        "highlighted: 10.0, 6.9 <-> 10.0, 5.0",
        "highlighted: 10.0, 5.0 <-> 8.0, 5.0",
        "highlighted: 8.0, 5.0 <-> 8.0, 4.0",
        "highlighted: 10.0, 0.3 <-> 10.0, -1.0",
        "highlighted: 10.0, -1.0 <-> 7.0, -1.0",
        "highlighted: 6.9, -6.9 <-> 1.0, -6.9",
        "highlighted: 1.0, -6.9 <-> 1.0, -3.5",
        "highlighted: 2.0, -2.0 <-> 2.0, -1.0",
        "highlighted: 2.0, -1.0 <-> 1.0, -1.0",
        "highlighted: 1.0, -1.0 <-> 1.0, 0.0",
        "highlighted: 1.0, 0.0 <-> 0.2, 0.0",
        "highlighted: 5.0, 4.0 <-> 5.0, 4.0", // these 2 lines may be weird, but it
        "highlighted: 5.0, 4.1 <-> 5.0, 5.0", // is only due to precision approximation, it's ok
        "highlighted: 5.0, 5.0 <-> 2.0, 5.0",
        "highlighted: 2.0, 5.0 <-> 2.0, 6.9",
    };

    CheckAndQuit(ss, expected);
}

TEST(CheckBigMapMergedWallsAgainAndAgain)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::CALCULATION_ALGO;

    //create the map
    AddSpot("spot_1", glm::vec2(-4.2889, 1.1222));
    AddSpot("spot_2", glm::vec2(6.3778, 4.6111));
    AddSpot("spot_3", glm::vec2(6.0667, -5.7889));
    AddSpot("spot_4", glm::vec2(-6.2444, -3.3667));

    AddWall("block1", glm::vec2(-10, 1), glm::vec2(-8, 1), false);
    AddWall("block2", glm::vec2(-8, 1), glm::vec2(-8, -1), false);
    AddWall("block3", glm::vec2(-10, -1), glm::vec2(-8, -1), false);
    AddWall("block4", glm::vec2(-10, -5), glm::vec2(-9, -5), false);
    AddWall("block5", glm::vec2(-9, -5), glm::vec2(-9, -7), false);
    AddWall("block6", glm::vec2(-1, -7), glm::vec2(-1, -2), false);
    AddWall("block7", glm::vec2(-2, -2), glm::vec2(-1, -2), false);
    AddWall("block8", glm::vec2(-2, -2), glm::vec2(-2, -1), false);
    AddWall("block9", glm::vec2(-2, -1), glm::vec2(-1, -1), false);
    AddWall("block10", glm::vec2(-1, -1), glm::vec2(-1, 0), false);
    AddWall("block11", glm::vec2(-1, 0), glm::vec2(1, 0), false);
    AddWall("block12", glm::vec2(1, -1), glm::vec2(1, 0), false);
    AddWall("block13", glm::vec2(2, -1), glm::vec2(1, -1), false);
    AddWall("block14", glm::vec2(2, -2), glm::vec2(2, -1), false);
    AddWall("block15", glm::vec2(2, -2), glm::vec2(1, -2), false);
    AddWall("block16", glm::vec2(1, -7), glm::vec2(1, -2), false);
    AddWall("block17", glm::vec2(10, -2), glm::vec2(8, -2), false);
    AddWall("block18", glm::vec2(8, -2), glm::vec2(8, -3), false);
    AddWall("block19", glm::vec2(8, -3), glm::vec2(7, -3), false);
    AddWall("block20", glm::vec2(7, -3), glm::vec2(7, -1), false);
    AddWall("block21", glm::vec2(7, -1), glm::vec2(10, -1), false);
    AddWall("block22", glm::vec2(10, 4), glm::vec2(8, 4), false);
    AddWall("block23", glm::vec2(8, 4), glm::vec2(8, 5), false);
    AddWall("block24", glm::vec2(8, 5), glm::vec2(10, 5), false);
    AddWall("block25", glm::vec2(2, 5), glm::vec2(2, 7), false);
    AddWall("block26", glm::vec2(-3, 7), glm::vec2(-3, 4), false);
    AddWall("block27", glm::vec2(-3, 4), glm::vec2(5, 4), false);
    AddWall("block28", glm::vec2(5, 4), glm::vec2(5, 5), false);
    AddWall("block29", glm::vec2(5, 5), glm::vec2(2, 5), false);


    //ensure that multiple updates does not broke the system
    theSpotSystem.PrepareAlgorithm();
    theSpotSystem.Update(1);
    theSpotSystem.Update(1);

    ss.str("");
    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "totalHighlightedDistance2Objective=144.8 and totalHighlightedDistance2Done=105.0",
        "highlighted: 8.0, 4.0 <-> 10.0, 4.0",
        "highlighted: -1.0, -1.0 <-> -2.0, -1.0",
        "highlighted: -8.0, 1.0 <-> -10.0, 1.0",
        "highlighted: 2.0, 5.8 <-> 2.0, 6.9",
        "highlighted: 10.0, 6.9 <-> 10.0, 5.5",
        "highlighted: 10.0, 4.0 <-> 10.0, -1.0",
        "highlighted: 10.0, -1.0 <-> 7.0, -1.0",
        "highlighted: 2.0, -1.0 <-> 1.0, -1.0",
        "highlighted: 1.0, -1.0 <-> 1.0, 0.0",
        "highlighted: 1.0, 0.0 <-> -1.0, 0.0",
        "highlighted: 1.0, -2.0 <-> 2.0, -2.0",
        "highlighted: 2.0, -2.0 <-> 2.0, -1.0",
        "highlighted: 5.0, 4.0 <-> 5.0, 5.0",
        "highlighted: 8.0, 5.0 <-> 8.0, 4.0",
        "highlighted: 7.0, -1.0 <-> 7.0, -3.0",
        "highlighted: 7.0, -3.0 <-> 8.0, -3.0",
        "highlighted: 8.7, -2.0 <-> 10.0, -2.0",
        "highlighted: 10.0, -2.0 <-> 10.0, -6.9",
        "highlighted: 10.0, -6.9 <-> 1.0, -6.9",
        "highlighted: 1.0, -6.9 <-> 1.0, -2.0",
        "highlighted: -10.0, -1.0 <-> -8.0, -1.0",
        "highlighted: -8.0, -1.0 <-> -8.0, 1.0",
        "highlighted: -10.0, 1.0 <-> -10.0, 6.9",
        "highlighted: -10.0, 6.9 <-> -3.0, 6.9",
        "highlighted: -3.0, 6.9 <-> -3.0, 4.0",
        "highlighted: -3.0, 4.0 <-> 5.0, 4.0",
        "highlighted: 2.0, 6.9 <-> 10.0, 6.9",
        "highlighted: -1.0, 0.0 <-> -1.0, -0.4",
        "highlighted: -2.0, -1.0 <-> -2.0, -2.0",
        "highlighted: -2.0, -2.0 <-> -1.0, -2.0",
        "highlighted: -1.0, -2.0 <-> -1.0, -6.9",
        "highlighted: -1.0, -6.9 <-> -9.0, -6.9",
        "highlighted: -9.0, -6.9 <-> -9.0, -5.0",
        "highlighted: -9.0, -5.0 <-> -10.0, -5.0",
        "highlighted: -10.0, -5.0 <-> -10.0, -1.0",
   };

    CheckAndQuit(ss, expected);
}
#endif
TEST(CheckDoubleFaceWall)
{
    std::stringstream ss;
    Init(ss);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::CALCULATION_ALGO;

    //create the map
    AddSpot("spot1", glm::vec2(0, 0));
    AddSpot("spot2", glm::vec2(0, 2));

    AddWall("block1", glm::vec2(-2, 1), glm::vec2(2, 1), true);

    //ensure that multiple updates does not broke the system
    theSpotSystem.PrepareAlgorithm();
    theSpotSystem.Update(1);
    theSpotSystem.Update(1);

    ss.str("");
    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "totalHighlightedDistance2Objective=75.8 and totalHighlightedDistance2Done=75.8",
        "highlighted: -2.0, 1.0 <-> 2.0, 1.0",
        "highlighted: 10.0, -6.9 <-> -10.0, -6.9",
        "highlighted: -10.0, 6.9 <-> 10.0, 6.9",
        "highlighted: 10.0, 6.9 <-> 10.0, -6.9",
        "highlighted: 2.0, 1.0 <-> -2.0, 1.0",
        "highlighted: -10.0, -6.9 <-> -10.0, 6.9",
    };

    CheckAndQuit(ss, expected);
}


#endif

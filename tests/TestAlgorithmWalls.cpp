#include <UnitTest++.h>

#include "systems/SpotSystem.h"
#include "systems/BlockSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include <glm/gtx/vector_angle.hpp>
#include <fstream>

#if 0
    CHECK_EQUAL(a, b);
    CHECK_CLOSE(a, b, 0.001);
#endif

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

static void Init(std::fstream & of) {
    SpotSystem::CreateInstance();
    BlockSystem::CreateInstance();
    RenderingSystem::CreateInstance(); //needed because we do some stuff with drawSomething class..
    TransformationSystem::CreateInstance();

    //write the output in a random generated temporary file
    char *tmpname = strdup("/tmp/tmpfileXXXXXX");
    mkstemp(tmpname);
    of.open(tmpname);

    theSpotSystem.outputStream = of.rdbuf();
}

static void CheckAndQuit(std::fstream & of, const std::vector<std::string> & expected) {
    //go back to the start of file, and compare result line by line
    of.seekp(0);
    std::string line;
    for (unsigned i = 0; i < expected.size(); ++i) {
        CHECK(of.good());
        getline(of, line);
        CHECK_EQUAL(expected[i], line);
    }
    of.close();

    SpotSystem::DestroyInstance();
    BlockSystem::DestroyInstance();
    RenderingSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
}

TEST(CheckWhenEmpty)
{
    std::fstream of;
    Init(of);

    //choose the flags
    theSpotSystem.FLAGS_ENABLED = SpotSystem::POINTS_ORDER;

    //create the map
    AddSpot("spot1", glm::vec2(0.));

    //do the algorithm
    theSpotSystem.Update(1);

    std::vector<std::string> expected = {
        "1. wall middle left (first point)",
        "2. wall top left",
        "3. top right",
        "4. wall bottom right",
        "5. wall bottom left",
    };

    CheckAndQuit(of, expected);
}

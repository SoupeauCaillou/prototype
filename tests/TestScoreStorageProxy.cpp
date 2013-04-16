#include <UnitTest++.h>
#include "util/ScoreStorageProxy.h"

TEST(TestGetterAndSetter)
{
    ScoreStorageProxy ssp;

    ssp.pushAnElement();
    ssp.setValue("name", "jose");
    ssp.setValue("points", "100");

    CHECK_EQUAL(ssp.getValue("name"), "jose");
}

TEST(TestEqualityWithRealScore)
{
    Score s;
    s.name = "jose";
    s.points = 100;

    ScoreStorageProxy ssp;
    ssp.pushAnElement();
    ssp.setValue("name", "jose");
    ssp.setValue("points", "100");

    CHECK_EQUAL(ssp._queue.front(), s);
}

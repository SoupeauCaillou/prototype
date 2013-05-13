#pragma once

#include "systems/System.h"

#include "base/Frequency.h"

#include <glm/glm.hpp>

struct DCAComponent {
    DCAComponent() : shoot(false), targetPoint(glm::vec2(1.f, 0.f)), fireRate(1.), dispersion(1.f),
     puissance(100.f), maximalDistanceForActivation(5), fireMode(EFireMode::FULL_AUTO), burstBulletCount(0), burstRestTime(1.f),
     owner(0), turret(0) {}

     bool shoot;

    //point where the DCA should aim
	glm::vec2 targetPoint;

    // in [0; +oo[, number of bullets per sec
	Frequency<float> fireRate;

    // in radians (R), dispersion angle from the direction axis. Bullets will be in [- angle ; +angle]
    // using gaussian distribution (deviation = this / 3)
    float dispersion;

    // in [0; +oo[, ejection speed of the bullets
    float puissance;

    // in [0; +oo[, maximal distance to activate the DCA with click
    float maximalDistanceForActivation;

    enum EFireMode {
        FULL_AUTO,
        BURST
    };

    EFireMode fireMode;

    // --ONLY BURST FIREMODE-- number of bullets fired for the current burst
    int burstBulletCount;
    // --ONLY BURST FIREMODE-- time between 2 consecutives burst /!\ it depends on the firerate of course
    // this value will represent real time when the firerate will be set to 1
    float burstRestTime;

    // who is the owner of this cannon ?
    Entity owner, turret;
};

#define theDCASystem DCASystem::GetInstance()
#define DCA(e) theDCASystem.Get(e)

UPDATABLE_SYSTEM(DCA)
};

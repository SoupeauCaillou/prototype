#pragma once

#include "systems/System.h"

#include "base/Frequency.h"

#include <glm/glm.hpp>

struct DCAComponent {
    DCAComponent() : targetPoint(glm::vec2(1.f, 0.f)), fireRate(1.), dispersion(1.f), puissance(100.f), maximalDistanceForActivation(5) {}

    //point where the DCA should aim
	glm::vec2 targetPoint;

    // in [0; +oo[, number of bullets per sec
	Frequency<float> fireRate;

    // in radians (R), dispersion angle from the direction axis. Bullets will be in [- angle/2 ; +angle/2]
    float dispersion;

    // in [0; +oo[, ejection speed of the bullets
    float puissance;

    // in [0; +oo[, maximal distance to activate the DCA with click
    float maximalDistanceForActivation;
};

#define theDCASystem DCASystem::GetInstance()
#define DCA(e) theDCASystem.Get(e)

UPDATABLE_SYSTEM(DCA)
};

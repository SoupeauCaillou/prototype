#pragma once

#include "systems/System.h"

struct ParachuteComponent {
	ParachuteComponent(): frottement(1.f) {
      /*  vect[0] = theEntityManager.CreateEntity("debug1",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("vectdebug"));
        vect[1] = theEntityManager.CreateEntity("debug2",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("vectdebug"));

         vect[2] = theEntityManager.CreateEntity("debug3",
                EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("vectdebug"));*/
    };
	float frottement;

    std::vector<glm::vec2> damages;

    //Entity vect[3];
};

#define theParachuteSystem ParachuteSystem::GetInstance()
#define PARACHUTE(e) theParachuteSystem.Get(e)

UPDATABLE_SYSTEM(Parachute)

public:
    void destroyParachute(Entity parachute);
};

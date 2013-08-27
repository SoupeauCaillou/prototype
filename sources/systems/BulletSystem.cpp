#include "BulletSystem.h"

#include "systems/CollisionSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/AutoDestroySystem.h"
#include "systems/OrcSystem.h"

INSTANCE_IMPL(BulletSystem);

BulletSystem::BulletSystem() : ComponentSystemImpl<BulletComponent>("Bullet") {

}

void BulletSystem::DoUpdate(float dt) {
	std::vector<Entity> toDelete;
	for (auto it = components.begin(); it!=components.end(); ++it) {
		Entity bullet = it->first;
		auto* ac = COLLISION(bullet);

		if(ac->collidedWithLastFrame > 0) {
            Entity hit = theEntityManager.CreateEntityFromTemplate("ingame/hit");
			TRANSFORM(hit)->position = TRANSFORM(bullet)->position;
			PARTICULE(hit)->initialColor = PARTICULE(hit)->finalColor = Interval<Color>(RENDERING(ac->collidedWithLastFrame)->color);

			toDelete.push_back(bullet);

			auto* oc = theOrcSystem.Get(ac->collidedWithLastFrame, false);
			if (oc) {
				COLLISION(ac->collidedWithLastFrame)->collideWith = 0;
				PARTICULE(ac->collidedWithLastFrame)->duration = 1;
				AUTO_DESTROY(ac->collidedWithLastFrame)->type = AutoDestroyComponent::LIFETIME;	
				toDelete.push_back(oc->weapon);
			}
		}
	}

	std::for_each(toDelete.begin(), toDelete.end(), [] (Entity e) {
		theEntityManager.DeleteEntity(e);
	});
}

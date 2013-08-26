#include "BulletSystem.h"

#include "systems/CollisionSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ParticuleSystem.h"

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
		}
	}

	std::for_each(toDelete.begin(), toDelete.end(), [] (Entity e) {
		theEntityManager.DeleteEntity(e);
	});
}

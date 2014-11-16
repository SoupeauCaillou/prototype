#include "BulletSystem.h"

#include "systems/CollisionSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/PhysicsSystem.h"

#include "util/Random.h"
#include "systems/UnitSystem.h"

INSTANCE_IMPL(BulletSystem);

BulletSystem::BulletSystem() : ComponentSystemImpl<BulletComponent>(HASH("Bullet", 0xca94697f)) {

}

void BulletSystem::DoUpdate(float dt) {
std::vector<Entity> toDelete;

FOR_EACH_ENTITY_COMPONENT(Bullet, bullet, bc)
    auto* ac = COLLISION(bullet);
    auto* cc = COLLISION(bullet);
    if (cc->rayTestDone) {
        if (cc->collision.count) {
            auto* a = theAnchorSystem.Get(cc->collision.with[0], false);
            if (a && a->parent) {
                Entity head = a->parent;
                Entity body = ANCHOR(head)->parent;
                auto* unit = UNIT(ANCHOR(body)->parent);
                /* kill unit */
                ANCHOR(unit->body)->parent = 0;
                TRANSFORM(unit->body)->rotation = TRANSFORM(bullet)->rotation;
                TRANSFORM(unit->body)->size.x = 1.3;
                TRANSFORM(unit->body)->z = 0.2;

                ANCHOR(unit->head)->position = glm::vec2(Random::Float(0.7f, 0.85f), Random::Float(-0.1f, 0.1f));
                for (int i=0; i<2; i++) {
                    ANCHOR(unit->weapon[i])->position = glm::vec2(Random::Float(-0.6, -0.2), Random::Float(-1.0f, -1.5f));
                    if (i == 1) ANCHOR(unit->weapon[i])->position.y = -ANCHOR(unit->weapon[i])->position.y;
                    ANCHOR(unit->weapon[i])->rotation = Random::Float(-2.5f, -0.2f);
                }

                RENDERING(unit->body)->color = Color(158.0f / 255, 158.0f / 255, 77.0f / 255.0f);
                RENDERING(unit->head)->color = Color(158.0f / 255, 77.0f / 255, 158.0f / 255.0f);

                theEntityManager.DeleteEntity(unit->hitzone);
                unit->hitzone = 0;
            }
        Entity b = theEntityManager.CreateEntityFromTemplate("bullet_ray");
        glm::vec2 diff = cc->collision.at[0] - TRANSFORM(bullet)->position;
        TRANSFORM(b)->size.x = glm::length(diff);
        TRANSFORM(b)->position = (TRANSFORM(bullet)->position + cc->collision.at[0]) * 0.5f;
        TRANSFORM(b)->rotation = glm::atan(diff.y, diff.x);
      }

  toDelete.push_back(bullet);
}
}

std::for_each(toDelete.begin(), toDelete.end(), [] (Entity e) {
  theEntityManager.DeleteEntity(e);
});
}

#include "BulletSystem.h"

#include "systems/CollisionSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/PhysicsSystem.h"

#include "util/Random.h"
#include "systems/UnitSystem.h"
#include "systems/WeaponSystem.h"

INSTANCE_IMPL(BulletSystem);

BulletSystem::BulletSystem() : ComponentSystemImpl<BulletComponent>(HASH("Bullet", 0xca94697f)) {

}

void BulletSystem::DoUpdate(float dt) {
    std::vector<Entity> toDelete;

    FOR_EACH_ENTITY_COMPONENT(Bullet, bullet, bc)
        auto* cc = COLLISION(bullet);
        if (cc->rayTestDone) {
            if (cc->collision.count) {
                auto* a = theAnchorSystem.Get(cc->collision.with[0], false);
                if (a && a->parent) {
                    Entity head = a->parent;
                    Entity body = ANCHOR(head)->parent;
                    auto* unit = UNIT(ANCHOR(body)->parent);
                    /* kill unit */
                    unit->alive = false;

                    ANCHOR(unit->body)->parent = 0;
                    TRANSFORM(unit->body)->rotation = TRANSFORM(bullet)->rotation;
                    TRANSFORM(unit->body)->size.x = 1.3;
                    TRANSFORM(unit->body)->z = 0.2;

                    ANCHOR(unit->head)->position = glm::vec2(Random::Float(0.7f, 0.85f), Random::Float(-0.1f, 0.1f));
                    for (int i=0; i<2; i++) {
                        ANCHOR(unit->weapon[i])->position = glm::vec2(Random::Float(-0.6, -0.2), Random::Float(-1.0f, -1.5f));
                        if (i == 1) ANCHOR(unit->weapon[i])->position.y = -ANCHOR(unit->weapon[i])->position.y;
                        ANCHOR(unit->weapon[i])->rotation = Random::Float(-2.5f, -0.2f);
                        WEAPON(unit->weapon[i])->fire = false;
                    }

                    RENDERING(unit->body)->color = Color(158.0f / 255, 158.0f / 255, 77.0f / 255.0f);
                    RENDERING(unit->head)->color = Color(158.0f / 255, 77.0f / 255, 158.0f / 255.0f);

                    theEntityManager.DeleteEntity(unit->hitzone);
                    unit->hitzone = 0;
                }

                {
                    // spawn debris
                    const glm::vec2 direction = a ?
                        glm::rotate(glm::vec2(-1.0f, 0.0f), TRANSFORM(bullet)->rotation) :
                        CollisionSystem::collisionPointToNormal(cc->collision.at[0], TRANSFORM(cc->collision.with[0]));
                    const Color c = a ?
                        Color(1, 0, 0) :
                        RENDERING(cc->collision.with[0])->color;

                    int count = Random::Int(7, 12);
                    for (int i=0; i<count; i++) {
                        Entity d = theEntityManager.CreateEntityFromTemplate("debris");
                        RENDERING(d)->color = c * Random::Float(0.8f, 1.0f);
                        TRANSFORM(d)->position = cc->collision.at[0];
                        TRANSFORM(d)->z = TRANSFORM(cc->collision.with[0])->z;
                        PHYSICS(d)->addForce(glm::rotate(direction * Random::Float(150.0f, 450.0f), Random::Float(-1, 1)), TRANSFORM(d)->size * Random::Float(-2, 2), 0.016);
                    }
                }

                Entity b = theEntityManager.CreateEntityFromTemplate("bullet_ray");
                glm::vec2 diff = cc->collision.at[0] - TRANSFORM(bullet)->position;
                TRANSFORM(b)->size.x = glm::length(diff);
                TRANSFORM(b)->position = (TRANSFORM(bullet)->position + cc->collision.at[0]) * 0.5f;
                TRANSFORM(b)->rotation = glm::atan(diff.y, diff.x);
            }

            toDelete.push_back(bullet);
        }
    END_FOR_EACH()

    std::for_each(toDelete.begin(), toDelete.end(), [] (Entity e) {
      theEntityManager.DeleteEntity(e);
    });
}

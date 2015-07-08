#include "VisionSystem.h"

#include "base/PlacementHelper.h"
#include "systems/TransformationSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"

#include "util/Draw.h"
#include "glm/gtx/norm.hpp"
#include "util/SerializerProperty.h"
#include "base/EntityManager.h"
#include <algorithm>

#include "base/TouchInputManager.h"

INSTANCE_IMPL(VisionSystem);

VisionSystem::VisionSystem()
    : ComponentSystemImpl<VisionComponent>(HASH("Vision", 0x32d754ca)) {
    VisionComponent vc;
    componentSerializer.add(
        new Property<float>(HASH("fov", 0xf7dddc94), OFFSET(fov, vc), 0.001f));
    componentSerializer.add(new Property<int>(
        HASH("collide_with", 0x6b658240), OFFSET(collideWith, vc), 0));
}

static void createRays(VisionSystem::RayDistance* out, int count) {
    for (int i = 0; i < count; i++) {
        Entity r =
            theEntityManager.CreateEntity(HASH("__/vision_ray", 0xbe930763));
        ADD_COMPONENT(r, Transformation);
        ADD_COMPONENT(r, Collision);
        COLLISION(r)->ray.is = true;
        COLLISION(r)->ray.maxCollision = 2;
        COLLISION(r)->collideWith = 1;
        out->e = r;
        out++;
    }
}

static inline glm::vec2* computeVertices(const TransformationComponent* tc,
                                         glm::vec2* out) {
    const glm::vec2& hSize = tc->size * 0.50f;
    glm::vec2 s1(glm::rotate(hSize, tc->rotation));
    glm::vec2 s2(glm::rotate(glm::vec2(hSize.x, -hSize.y), tc->rotation));

    *(out + 0) = tc->position + s1;
    *(out + 1) = tc->position - s2;
    *(out + 2) = tc->position - s1;
    *(out + 3) = tc->position + s2;
    return out + 4;
}

void VisionSystem::DoUpdate(float) {
    /* naively build vertex list of all colliding object */
    const auto& entities = theCollisionSystem.RetrieveAllEntityWithComponent();

    const size_t s = entities.size();
    std::vector<Entity> valid;

    int resultIndex = 0;
    if (!vertices.empty()) {
        FOR_EACH_ENTITY_COMPONENT(Vision, e, vc)
        /* read rays results from last frame */
        vc->vertices.pos = &vertices[resultIndex];
        vc->vertices.count = 0;

        int first = vc->_rayStartIndex;
        int latest = first + vc->_rayCount - 1;

        if (latest < first) {
            continue;
        }

        Color color[] = {Color(1, 0, 0), Color(0, 1, 0), Color(0, 0, 1)};

        const auto pos = TRANSFORM(rays[0].e)->position;
        Entity prevEntityHit = 0;
        bool wasFirstHit = false;
        for (int i = first; i <= latest; i++) {
            const auto* cc = COLLISION(rays[i].e);

            {
                float d0 = glm::distance2(pos, cc->collision.at[0]);
                bool is_corner = (glm::abs(d0 - rays[i].d) < 0.001f);
                int start = 0, dec = 1;

                if (is_corner && cc->collision.count > 1) {
                    Entity hit1 = cc->collision.with[0];
                    Entity hit2 = cc->collision.with[1];
                    if (hit1 != hit2) {
                        glm::vec2 diff =
                            TRANSFORM(cc->collision.with[0])->position - pos;
                        float angle = glm::atan(diff.y, diff.x);
                        if ((angle - TRANSFORM(rays[i].e)->rotation) > 0) {
                            dec = -1;
                            start = 1;
                        }
                    }
                }

                for (int i = start; i >= 0 && i < cc->collision.count;
                     i += dec) {

                    Entity hit = cc->collision.with[i];

                    /* ignore 2nd hit on the same entity */
                    if (i != start) {
                        if (hit == cc->collision.with[start])
                            break;
                    }
                    if (hit == prevEntityHit) {
                        if (!wasFirstHit /*&& !is_corner*/) {
                            vertices[resultIndex + vc->vertices.count - 1] =
                                cc->collision.at[i];
                        } else {
                            wasFirstHit = false /*|| is_corner*/;
                            vertices[resultIndex + vc->vertices.count++] =
                                cc->collision.at[i];
                        }
                    } else {
                        prevEntityHit = hit;
                        wasFirstHit = true;
                        vertices[resultIndex + vc->vertices.count++] =
                            cc->collision.at[i];
                    }
                }
                continue;
            }

            /* collision #0 can be either:
                * the corner/vertex aimed by the ray: in this case we want to
             * keep this hit + the next hit on a different entity
                * a middle of a edge: ignored because it's a useless
             * intermediate point (eg when aiming the back corner)
                * before corner
            */
            float d0 = glm::distance2(pos, cc->collision.at[0]);

            /* case 1: corner hit */
            if (glm::abs(d0 - rays[i].d) < 0.001f) {
                Entity hit1 = cc->collision.with[0];

                int nextHitIndex = 1;
                /*while (nextHitIndex < cc->collision.count &&
                    cc->collision.with[nextHitIndex] == hit1) {
                    nextHitIndex++;
                }*/

                if (nextHitIndex == cc->collision.count) {
                    /* no further hit, keep track of 1st hit */
                    vertices[resultIndex + vc->vertices.count] =
                        cc->collision.at[0];
                    vc->vertices.count++;

                    prevEntityHit = hit1;
                } else {
                    Entity hit2 = cc->collision.with[nextHitIndex];

                    if (hit2 != hit1) {
                        if (hit1 == prevEntityHit) {
                            vertices[resultIndex + vc->vertices.count] =
                                cc->collision.at[0];
                            vertices[resultIndex + vc->vertices.count + 1] =
                                cc->collision.at[nextHitIndex];

                            prevEntityHit = hit2;
                        } else {
                            vertices[resultIndex + vc->vertices.count] =
                                cc->collision.at[nextHitIndex];
                            vertices[resultIndex + vc->vertices.count + 1] =
                                cc->collision.at[0];

                            prevEntityHit = hit1;
                        }
                        vc->vertices.count += 2;
                    } else {
                        /* ray went through entity, so keep the first hit */
                        vertices[resultIndex + vc->vertices.count] =
                            cc->collision.at[0];
                        vc->vertices.count++;
                        prevEntityHit = hit1;
                    }
                }
            }
        }

#if 1
        /* draw...*/
        static std::vector<Entity> triangles;

        if ((int)theTransformationSystem.shapes.size() <
            ((int)Shape::Count + vc->vertices.count)) {
            theTransformationSystem.shapes.resize((int)Shape::Count +
                                                  vc->vertices.count);
            int old = (int)triangles.size();
            triangles.resize(vc->vertices.count);
            for (int i = old; i < vc->vertices.count; i++) {
                Entity e =
                    theEntityManager.CreateEntity(HASH("tri", 0x452977f5));
                ADD_COMPONENT(e, Transformation);
                ADD_COMPONENT(e, Rendering);
                triangles[i] = e;
                RENDERING(e)
                    ->flags =
                    RenderingFlags::NonOpaque | RenderingFlags::NoCulling;
            }
        }
        for (int i = 0; i < vc->vertices.count - 1; i++) {
            char id[10];
            sprintf(id, "%d", i);
            Draw::Point(vertices[resultIndex + i], color[1], id);
            Polygon tri = Polygon::create(Shape::Triangle);
            tri.vertices[0] = glm::vec2(0.0f);
            tri.vertices[1] = vertices[resultIndex + i] - pos;
            tri.vertices[2] =
                vertices[resultIndex + (i + 1) % vc->vertices.count] - pos;
            theTransformationSystem.shapes[i + (int)Shape::Count] = tri;

            Entity e = triangles[i];
            TRANSFORM(e)->position = pos;
            RENDERING(e)->show = true;

            float t = (float)i / vc->vertices.count;
            glm::vec3 a(0.5f);
            glm::vec3 b(0.5f);
            glm::vec3 c(1.0f, 0.7f, 0.4f);
            glm::vec3 d(0.0f, 0.15f, 0.20f);

            glm::vec3 col =
                a + b * glm::cos(2 * glm::pi<float>() * (c * t + d));
            RENDERING(e)->color = Color(col.x, col.y, col.z, 0.7f);
            TRANSFORM(e)->shape = (Shape::Enum)(i + (int)Shape::Count);
        }
        for (int i = vc->vertices.count; i < (int)triangles.size(); i++) {
            RENDERING(triangles[i])->show = false;
        }
#endif
        resultIndex += vc->vertices.count;
        END_FOR_EACH()
    }

    /* update rays */
    std::vector<glm::vec2> wallEnds(s * 4);
    glm::vec2* ptr = &wallEnds[0];

    for (size_t i = 0; i < s; i++) {
        if (COLLISION(entities[i])->group == 1) {
            const auto* tc = TRANSFORM(entities[i]);
            ptr = computeVertices(tc, ptr);
            valid.push_back(entities[i]);
        }
    }
    wallEnds.resize(ptr - &wallEnds[0]);
    int raycount = wallEnds.size() * entityWithComponent.size();

    /* spawn new rays */
    if ((int)rays.size() < raycount) {
        int oldSize = rays.size();
        rays.resize(raycount);
        createRays(&rays[oldSize], raycount - oldSize);
        /* we'll keep at most 2 vertices per ray */
        vertices.resize(raycount * 2);
    }

    int rayIndex = 0;
    const size_t rayCountPerEntity = wallEnds.size();

    FOR_EACH_ENTITY_COMPONENT(Vision, e, vc)
    int start = vc->_rayStartIndex = rayIndex;
    vc->_rayCount = rayCountPerEntity;
    const auto& position = TRANSFORM(e)->position;

    for (size_t i = 0; i < rayCountPerEntity; i++) {
        Entity ray = rays[start + i].e;
        auto* tc = TRANSFORM(ray);
        tc->position = position;
        glm::vec2 diff =
            /*theTouchInputManager.getTouchLastPosition() */ wallEnds[i] -
            position;
        tc->rotation = glm::atan(diff.y, diff.x);
        auto* cc = COLLISION(ray);
        cc->ray.testDone = false;
        // cc->ignore = valid[i / 4];

        rays[start + i].d = glm::distance2(position, wallEnds[i]);
        // break;
    }

    /* sort rays by angle */
    std::sort(rays.begin() + start,
              rays.begin() + start + rayCountPerEntity,
              [](RayDistance a, RayDistance b) -> bool {
                  return TRANSFORM(a.e)->rotation < TRANSFORM(b.e)->rotation;
              });

    rayIndex += rayCountPerEntity;
    END_FOR_EACH()
}

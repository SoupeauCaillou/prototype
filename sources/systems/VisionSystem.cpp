#include "VisionSystem.h"

#include "base/PlacementHelper.h"
#include "systems/TransformationSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"

#include "util/Draw.h"
#include "glm/gtx/norm.hpp"

INSTANCE_IMPL(VisionSystem);

VisionSystem::VisionSystem() : ComponentSystemImpl<VisionComponent>(HASH("Vision", 0x32d754ca)) {
    VisionComponent vc;
    componentSerializer.add(new Property<float>(HASH("fov", 0xf7dddc94), OFFSET(fov, vc), 0.001f));
    componentSerializer.add(new Property<int>(HASH("collide_with", 0x6b658240), OFFSET(collideWith, vc), 0));
}

static void createRays(VisionSystem::RayDistance* out, int count) {
    for (int i=0; i<count; i++) {
        Entity r = theEntityManager.CreateEntity(HASH("__/vision_ray", 0xbe930763));
        ADD_COMPONENT(r, Transformation);
        ADD_COMPONENT(r, Collision);
        COLLISION(r)->ray.is = true;
        COLLISION(r)->ray.maxCollision = 3;
        COLLISION(r)->collideWith = 1;
        out->e = r;
        out++;
    }
}

static inline glm::vec2* computeVertices(const TransformationComponent* tc, glm::vec2* out) {
    const glm::vec2& hSize = tc->size * 0.50f;
    glm::vec2 s1(glm::rotate(hSize, tc->rotation));
    glm::vec2 s2(glm::rotate(glm::vec2(hSize.x, -hSize.y), tc->rotation));

    *(out+0) = tc->position + s1;
    *(out+1) = tc->position - s2;
    *(out+2) = tc->position - s1;
    *(out+3) = tc->position + s2;
    return out + 4;
}

void VisionSystem::DoUpdate(float) {
    /* naively build vertex list of all colliding object */
    const auto& entities = theCollisionSystem.RetrieveAllEntityWithComponent();

    const size_t s = entities.size();
    std::vector<Entity> valid;
    std::vector<glm::vec2> wallEnds(s * 4);
    glm::vec2* ptr = &wallEnds[0];

    for (size_t i=0; i<s; i++) {
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
        vertices.resize(raycount);
    }

    int resultIndex = 0;
    FOR_EACH_ENTITY_COMPONENT(Vision, e, vc)
        /* read rays results from last frame */
        vc->vertices.pos = &vertices[resultIndex];
        vc->vertices.count = 0;

        int first = vc->_rayStartIndex;
        int latest = first + vc->_rayCount - 1;

        Color color[] = {
            Color(1, 0, 0),
            Color(0, 1, 0),
            Color(0, 0, 1)};
        const auto& pos = TRANSFORM(rays[0].e)->position;
        for (int i=first; i<=latest; i++) {
            const auto* cc = COLLISION(rays[i].e);
            Entity col = 0;
            for (int j=0; j<cc->collision.count; j++) {
                float d = glm::distance2(pos, cc->collision.at[j]);

                if (d - rays[i].d < -0.1) {
                    break;
                    vertices[resultIndex + vc->vertices.count] = cc->collision.at[j];
                    vc->vertices.count++;
                    Draw::Point(cc->collision.at[j], Color(1, 0, 1));
                    break;
                }

                if (cc->collision.with[j] != col) {
                    vertices[resultIndex + vc->vertices.count] = cc->collision.at[j];
                    vc->vertices.count++;
                    std::stringstream a;
                    a << i << '.' << j << ':' << cc->collision.with[j] << '=' << col;
                    Draw::Point(cc->collision.at[j], color[j], a.str());
                    col = cc->collision.with[j];
                } else {
                    break;
                }
            }
        }
        resultIndex += vc->vertices.count;


#if 1
        /* draw...*/
        static std::vector<Entity> triangles;

        if ((int)theTransformationSystem.shapes.size() < ((int)Shape::Count + vc->vertices.count)) {
            theTransformationSystem.shapes.resize((int)Shape::Count + vc->vertices.count);
            int old = (int)triangles.size();
            triangles.resize(vc->vertices.count);
            for (int i=old; i<vc->vertices.count; i++) {
                Entity e = theEntityManager.CreateEntity(HASH("tri", 0x452977f5));
                ADD_COMPONENT(e, Transformation);
                ADD_COMPONENT(e, Rendering);
                triangles[i] = e;
            }
        }
        for (int i=0; i<vc->vertices.count; i++) {
            Polygon tri = Polygon::create(Shape::Triangle);
            tri.vertices[0] = glm::vec2(0.0f);
            tri.vertices[1] = vertices[i] - pos;
            tri.vertices[2] = vertices[(i + 1) % vc->vertices.count] - pos;
            theTransformationSystem.shapes[i + (int)Shape::Count] = tri;

            Entity e = triangles[i];
            TRANSFORM(e)->position = pos;
            RENDERING(e)->show = true;
            RENDERING(e)->flags = RenderingFlags::NoCulling;
            TRANSFORM(e)->shape = (Shape::Enum) (i + (int)Shape::Count);
        }
        for (int i=vc->vertices.count; i<(int)triangles.size();i++) {
            RENDERING(triangles[i])->show = false;
        }
#endif
    END_FOR_EACH()

    /* update rays */
    int rayIndex = 0;
    const size_t rayCountPerEntity = wallEnds.size();

    FOR_EACH_ENTITY_COMPONENT(Vision, e, vc)
        int start = vc->_rayStartIndex = rayIndex;
        vc->_rayCount = rayCountPerEntity;
        const auto& position = TRANSFORM(e)->position;

        for (size_t i=0; i<rayCountPerEntity; i++) {
            Entity ray = rays[start + i].e;
            auto* tc = TRANSFORM(ray);
            tc->position = position;
            glm::vec2 diff = wallEnds[i] - position;
            tc->rotation = glm::atan(diff.y, diff.x);
            auto* cc = COLLISION(ray);
            cc->ray.testDone = false;
            // cc->ignore = valid[i / 4];

            rays[start + i].d = glm::distance2(position, wallEnds[i]);
        }

        /* sort rays by angle */
        std::sort(rays.begin() + start, rays.begin() + start + rayCountPerEntity, [] (RayDistance a, RayDistance b) -> bool {
            return TRANSFORM(a.e)->rotation < TRANSFORM(b.e)->rotation;
        });

        rayIndex += rayCountPerEntity;
    END_FOR_EACH()
}


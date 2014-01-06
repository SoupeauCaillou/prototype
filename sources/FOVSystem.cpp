/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "FOVSystem.h"

#include "systems/CollisionSystem.h"
#include "systems/TransformationSystem.h"

INSTANCE_IMPL(FOVSystem);

Entity createRay() {
    Entity r = theEntityManager.CreateEntity("__fov_ray");
    ADD_COMPONENT(r, Transformation);
    ADD_COMPONENT(r, Collision);
    COLLISION(r)->isARay = true;
    return r;
}

struct FloatApproxCompare {
    bool operator()(float a, float b) {
        return a < b;
    }
};

static float angleFromVector(const glm::vec2& from, const glm::vec2& to) {
    const glm::vec2 diff(to - from);
    return glm::atan2(diff.y, diff.x);
}

FOVSystem::FOVSystem() : ComponentSystemImpl<FOVComponent>("FOV") {
    FOVComponent tc;
    componentSerializer.add(new Property<int>("groups_impacting_fov", OFFSET(groupsImpactingFOV, tc)));
}

void FOVSystem::DoUpdate(float) {
    int raysUsed = 0;

    for (auto p: components) {
        const Entity e = p.first;
        auto* fc = p.second;

        fc->reference = TRANSFORM(e)->position;
        fc->polygons.clear();

        bool complete = true;
        // build polygons (triangles) from previous frame results (if any)
        if (fc->rayIndexes[0] >= 0) {
            Polygon triangle = Polygon::create(Shape::Triangle);
            triangle.vertices[0] = fc->reference;
            triangle.vertices[1] = COLLISION(rays[fc->rayIndexes[0]])->collisionAt;
            for (int i=fc->rayIndexes[0]; i<fc->rayIndexes[1]; i++) {
                if(!COLLISION(rays[i + 1])->rayTestDone) {
                    complete = false;
                    break;
                }
                triangle.vertices[2] = COLLISION(rays[i + 1])->collisionAt;
                fc->polygons.push_back(triangle);
                triangle.vertices[1] = triangle.vertices[2];
            }
            if (complete) {
                triangle.vertices[2] = COLLISION(rays[fc->rayIndexes[0]])->collisionAt;
                fc->polygons.push_back(triangle);
            }
        }

        if (complete) {
            fc->rayIndexes[0] = fc->rayIndexes[1] = -1;
        } else {
            continue;
        }

        if (fc->groupsImpactingFOV <= 0)
            continue;

        // compute new raycasts angles
        const auto& world = theCollisionSystem.worldSize;
        std::set<float, FloatApproxCompare> rayAngles;
        //   * world corners
        rayAngles.insert(angleFromVector(fc->reference, world * glm::vec2(-0.5, -0.5)));
        rayAngles.insert(angleFromVector(fc->reference, world * glm::vec2(-0.5, 0.5)));
        rayAngles.insert(angleFromVector(fc->reference, world * glm::vec2(0.5, -0.5)));
        rayAngles.insert(angleFromVector(fc->reference, world * glm::vec2(0.5, 0.5)));

        //   * 1 ray per object vertex
        theCollisionSystem.forEachECDo([this, fc, &rayAngles] (Entity c, CollisionComponent* cc) -> void {
            if (cc->group & fc->groupsImpactingFOV) {
                std::vector<glm::vec2> pos;
                TransformationSystem::appendVerticesTo(TRANSFORM(c), pos);
                for (int i=0; i<4; i++) {
                    rayAngles.insert(angleFromVector(fc->reference, pos[i]));
                }
            }
        });

        // fire raycasts
        while (rays.size() < (raysUsed + rayAngles.size())) {
            rays.push_back(createRay());
        }
        fc->rayIndexes[0] = raysUsed;
        for (auto ray: rayAngles) {
            auto* tc = TRANSFORM(rays[raysUsed]);
            tc->position = fc->reference;
            tc->rotation = ray;
            auto* cc = COLLISION(rays[raysUsed]);
            cc->collideWith = fc->groupsImpactingFOV;
            cc->rayTestDone = false;
            raysUsed++;
        }
        fc->rayIndexes[1] = raysUsed - 1;
    }
}


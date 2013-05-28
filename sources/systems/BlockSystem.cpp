#include "BlockSystem.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

INSTANCE_IMPL(BlockSystem);

BlockSystem::BlockSystem() : ComponentSystemImpl <BlockComponent>("Block") {
}


static int current = 0;
Entity drawPoint(const glm::vec2& position) {
    static std::vector<Entity> list;

    Entity vector;

    if (current == (int)list.size()) {
        vector = theEntityManager.CreateEntity("vector");
        ADD_COMPONENT(vector, Transformation);
        ADD_COMPONENT(vector, Rendering);

        list.push_back(vector);
    } else {
        vector = list[current];
    }

    TRANSFORM(vector)->size = glm::vec2(0.1f);
    TRANSFORM(vector)->position = position;

    TRANSFORM(vector)->z = 1;
    RENDERING(vector)->color = Color(1.,0.,0.);
    RENDERING(vector)->show = true;

    ++current;

    return vector;
}

void splitIntoSegments(const glm::vec2 & pointA, const glm::vec2 & pointB, std::list<std::pair<glm::vec2, glm::vec2>> & edges) {
    drawPoint(pointA);
    drawPoint(pointB);

    const float minimalDistanceBetweenPoints = 2.f;

    float length = glm::length(pointA - pointB);
    int intermediatePointsCount = length / minimalDistanceBetweenPoints;

    if (intermediatePointsCount > 0) {
        float step = length / intermediatePointsCount;

        edges.push_back(std::make_pair (pointA, pointA + step));
        edges.push_back(std::make_pair (pointB, pointB - step));

        for (int i = 1; i <= intermediatePointsCount; ++i)
        {
            float current = i * step / length;

            drawPoint(pointA + (pointB - pointA) * current);
            edges.push_back(std::make_pair (pointA, pointA + (pointB - pointA) * current));
        }
    } else {
        //add the whole edge if it was too small to be cut
        edges.push_back(std::make_pair (pointA, pointB));
    }
}

void BlockSystem::DoUpdate(float) {
    current = 0;
    std::list<std::pair<glm::vec2, glm::vec2>> edges;
    // LOGI(components.size());
    FOR_EACH_ENTITY(Block, e)
        TransformationComponent * tc = TRANSFORM(e);

        glm::vec2 points[4] = {
            tc->position - tc->size / 2.f, //NW
            tc->position + glm::vec2(tc->size.x, -tc->size.y) / 2.f, //NE
            tc->position - glm::vec2(tc->size.x, -tc->size.y) / 2.f, //SW
            tc->position + tc->size / 2.f //SE
        };

        splitIntoSegments(points[0], points[1], edges);
        splitIntoSegments(points[0], points[2], edges);
        splitIntoSegments(points[1], points[3], edges);
        splitIntoSegments(points[2], points[3], edges);
    }


}

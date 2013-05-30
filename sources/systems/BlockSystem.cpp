#include "BlockSystem.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "util/drawVector.h"
#include "util/IntersectionUtil.h"

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/projection.hpp>

#include <algorithm>

INSTANCE_IMPL(BlockSystem);

BlockSystem::BlockSystem() : ComponentSystemImpl <BlockComponent>("Block") {
}


static int currentDrawPointIndice = 0;
Entity drawPoint(const glm::vec2& position, const Color & color = Color(0.5, 0.5, 0.5)) {
    static std::vector<Entity> list;

    Entity vector;

    if (currentDrawPointIndice == (int)list.size()) {
        vector = theEntityManager.CreateEntity("vector");
        ADD_COMPONENT(vector, Transformation);
        ADD_COMPONENT(vector, Rendering);

        list.push_back(vector);
    } else {
        vector = list[currentDrawPointIndice];
    }

    TRANSFORM(vector)->size = glm::vec2(0.5f);
    TRANSFORM(vector)->position = position;

    TRANSFORM(vector)->z = 1;
    RENDERING(vector)->color = color;//Color(.5, currentDrawPointIndice * 1.f / list.size(), currentDrawPointIndice * 1.f / list.size());
    RENDERING(vector)->show = true;

    ++currentDrawPointIndice;

    return vector;
}

static int currentDrawEdgeIndice = 0;
Entity drawEdge(const glm::vec2& positionA, const glm::vec2& positionB) {
    static std::vector<Entity> list;

    Entity vector;

    if (currentDrawEdgeIndice == (int)list.size()) {
        vector = theEntityManager.CreateEntity("vector");
        ADD_COMPONENT(vector, Transformation);
        ADD_COMPONENT(vector, Rendering);

        list.push_back(vector);
    } else {
        vector = list[currentDrawEdgeIndice];
    }

    drawVector(positionA, glm::vec2(glm::length(positionB - positionA)), vector);

    ++currentDrawEdgeIndice;

    return vector;
}

void splitIntoSegments(const glm::vec2 & pointA, const glm::vec2 & pointB, std::list<std::pair<glm::vec2, glm::vec2>> & edges) {
    // drawPoint(pointA);
    // drawPoint(pointB);

    const float minimalDistanceBetweenPoints = .5f;

    float length = glm::length(pointA - pointB);
    int intermediatePointsCount = length / minimalDistanceBetweenPoints;

    if (intermediatePointsCount > 0) {
        float step = length / intermediatePointsCount;

        edges.push_back(std::make_pair (pointA, pointA + step));
        edges.push_back(std::make_pair (pointB, pointB - step));

        for (int i = 1; i <= intermediatePointsCount; ++i)
        {
            float current = i * step / length;

            // drawPoint(pointA + (pointB - pointA) * current);
            edges.push_back(std::make_pair (pointA, pointA + (pointB - pointA) * current));
        }
    } else {
        //add the whole edge if it was too small to be cut
        edges.push_back(std::make_pair (pointA, pointB));
    }
}

struct EnhancedPoint {
    EnhancedPoint() : position(0.), parent(0), edge1(0.), edge2(0.), name("unknown") {}
    EnhancedPoint(const glm::vec2& inp, Entity p, const glm::vec2& ine1, const glm::vec2& ine2, const std::string & iname) : position(inp), parent(p), edge1(ine1), edge2(ine2), name(iname) {}
    glm::vec2 position;

    Entity parent; //actually useless

    glm::vec2 edge1;
    glm::vec2 edge2;

    std::string name; //only debug
    bool operator< (const EnhancedPoint & ep) const {
        float firstAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(position));
        float secondAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(ep.position));
        return (firstAngle < secondAngle);
    }
};

// Return minimum distance between line segment vw and point p
// see http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
float distancePointToSegment(const glm::vec2 & v, const glm::vec2 & w, const glm::vec2 & p) {
    const float norm2 = glm::length2(w - v);

    glm::vec2 projectionOnSegment;

    // if v = w, this is not a segment!
    if (norm2 < 0.0001f) {
        projectionOnSegment = v;
    } else {
        const float t = glm::dot (p - v, w - v) / norm2;

        // Beyond the 'v' end of the segment
        if ( t < 0.f) {
            projectionOnSegment = v;
        // Beyond the 'w' end of the segment
        } else if (t  > 1.f) {
            projectionOnSegment = w;
         // Projection falls on the segment
        } else {
            projectionOnSegment = v + t * (w - v);
        }
    }

    drawPoint(projectionOnSegment, Color(1., 0., 0.));

    return glm::length(projectionOnSegment - p);
}


void BlockSystem::DoUpdate(float) {
    currentDrawPointIndice = 0;
    currentDrawEdgeIndice = 0;

    std::list<EnhancedPoint> points;

    FOR_EACH_ENTITY(Block, e)
        TransformationComponent * tc = TRANSFORM(e);

        glm::vec2 rectanglePoints[4] = {
            tc->position - glm::vec2(tc->size.x, -tc->size.y) / 2.f, //NW
            tc->position + tc->size / 2.f, //NE
            tc->position - tc->size / 2.f, //SW
            tc->position + glm::vec2(tc->size.x, -tc->size.y) / 2.f, //SE
        };
        points.push_back(EnhancedPoint(rectanglePoints[0], e, rectanglePoints[1], rectanglePoints[2], theEntityManager.entityName(e) + "- top left"));
        points.push_back(EnhancedPoint(rectanglePoints[1], e, rectanglePoints[0], rectanglePoints[3], theEntityManager.entityName(e) + "- top right"));
        points.push_back(EnhancedPoint(rectanglePoints[2], e, rectanglePoints[0], rectanglePoints[3], theEntityManager.entityName(e) + "- bottom left"));
        points.push_back(EnhancedPoint(rectanglePoints[3], e, rectanglePoints[1], rectanglePoints[2], theEntityManager.entityName(e) + "- bottom right"));
    }

    points.sort();


    // var endpoints;      # list of endpoints, sorted by angle
    // var open = [];      # list of walls, sorted by distance

    // loop over endpoints:
    //     remember which wall is nearest
    //     add any walls that BEGIN at this endpoint to 'walls'
    //     remove any walls that END at this endpoint from 'walls'

    //     SORT the open list

    //     if the nearest wall changed:
    //         fill the current triangle and begin a new one

    std::pair<glm::vec2, glm::vec2> nearestWall;

    std::list<std::pair<glm::vec2, glm::vec2>> walls;

    int i = 0;
    for (auto point : points) {
        drawPoint(point.position);
        LOGI(++i << " " << point.name << " " << glm::degrees(glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(point.position))));

        // Sauvegarde du mur le plus proche
        std::pair<glm::vec2, glm::vec2> nearestWall;
        if (walls.size() > 0) {
            nearestWall = walls.front();
        }
        LOGI("\tNearest wall is " << nearestWall.first << " <-> " << nearestWall.second);

        // On supprime tous les segments qui ont comme fin le point qu'on va rajouter (sinon ils y seront 2 fois)
        for (auto wall = walls.begin(); wall != walls.end();) {
            auto it = wall;
            //in case of deletion, avoid problem with iterators
            ++wall;
            // LOGI("\t\t\t" << glm::length2(glm::proj(glm::vec2(0.), (it->second - it->first))));

            if (glm::length2(it->second - point.position) < 0.0001f) {
                LOGI("Found position " << it->second << " as end position - erasing it");
                walls.erase(it);
            }
        }

        LOGI("\t\tAdding1 " << point.position << " <-> " << point.edge1);
        LOGI("\t\tAdding2 " << point.position << " <-> " << point.edge2);
        walls.push_back(std::make_pair(point.position, point.edge1));
        walls.push_back(std::make_pair(point.position, point.edge2));

        walls.sort([] (std::pair<glm::vec2, glm::vec2> & w1, std::pair<glm::vec2, glm::vec2> & w2) {
            const glm::vec2 pointOfView = glm::vec2(0.f, 0.f);

            return distancePointToSegment(w1.first, w1.second, pointOfView) < distancePointToSegment(w2.first, w2.second, pointOfView);
        });

        // Si on a changé de segment le plus proche, on plot un triangle
        if (walls.size() > 2 && glm::length2(walls.front().first - nearestWall.first) &&
            glm::length2(walls.front().second - nearestWall.second)> 0.0001f) {
            LOGI("New nearest wall! Drawing triangle - TODO");
        }
    }
/*
    for (auto edge : edges) {
        bool isIntersected = false;
        FOR_EACH_ENTITY(Block, e)
            TransformationComponent * tc = TRANSFORM(e);
            isIntersected = IntersectionUtil::lineRectangle(glm::vec2(0.), edge.first, tc->position, tc->size, tc->rotation)
            || IntersectionUtil::lineRectangle(glm::vec2(0.), edge.second, tc->position, tc->size, tc->rotation);
            if (isIntersected)
                break;
        }

        if (! isIntersected) {
//            drawEdge(edge.first, edge.second);
            drawPoint((edge.first + edge.second) / 2.f) ;
        }
    }*/
}

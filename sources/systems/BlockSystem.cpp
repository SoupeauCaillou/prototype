#include "BlockSystem.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "util/drawVector.h"
#include "util/IntersectionUtil.h"

#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/projection.hpp>

#include <algorithm>

INSTANCE_IMPL(BlockSystem);

BlockSystem::BlockSystem() : ComponentSystemImpl <BlockComponent>("Block") {
}

static int currentDrawPointIndice = 0;
static std::vector<Entity> drawPointList;
Entity drawPoint(const glm::vec2& position, const std::string name = "vector", const Color & color = Color(0.5, 0.5, 0.5)) {
    Entity vector;

    if (currentDrawPointIndice == (int)drawPointList.size()) {
        vector = theEntityManager.CreateEntity(name);
        ADD_COMPONENT(vector, Transformation);
        ADD_COMPONENT(vector, Rendering);

        drawPointList.push_back(vector);
    } else {
        vector = drawPointList[currentDrawPointIndice];
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
static std::vector<Entity> drawEdgeList;
Entity drawEdge(const glm::vec2& positionA, const glm::vec2& positionB, const Color & color = Color(0.3, 0.5, 0.8)) {

    Entity vector;

    if (currentDrawEdgeIndice == (int)drawEdgeList.size()) {
        vector = theEntityManager.CreateEntity("drawEdge vector");
        ADD_COMPONENT(vector, Transformation);
        ADD_COMPONENT(vector, Rendering);

        drawEdgeList.push_back(vector);
    } else {
        vector = drawEdgeList[currentDrawEdgeIndice];
    }

    drawVector(positionA, positionB - positionA, vector, color);
    RENDERING(vector)->texture = InvalidTextureRef;
    RENDERING(vector)->show = true;
    ++currentDrawEdgeIndice;

    return vector;
}

static int currentDrawTriangleColorIndice = 0;
static std::vector<Color> drawTriangleColorList;
void drawTriangle(const glm::vec2& pointOfView, const glm::vec2& first, const glm::vec2& second) {
    LOGI("triangle: '" << pointOfView << "' x '" << first << "' x '" << second << "'");

    if (currentDrawTriangleColorIndice == (int)drawTriangleColorList.size()) {
        drawTriangleColorList.push_back(Color::random());
        drawTriangleColorList[currentDrawTriangleColorIndice].a = 1.;
    }
    // drawEdge(pointOfView, first, drawTriangleColorList[currentDrawTriangleColorIndice]);
    // drawEdge(pointOfView, second, drawTriangleColorList[currentDrawTriangleColorIndice]);
    drawEdge(first, second, drawTriangleColorList[currentDrawTriangleColorIndice]);

    ++currentDrawTriangleColorIndice;
}


struct EnhancedPoint {
    EnhancedPoint() : position(0.), nextEdge1(0.), nextEdge2(0.), name("unknown") {}
    EnhancedPoint(const glm::vec2& inp, const glm::vec2& ine1, const glm::vec2& ine2, const std::string & iname)
    : position(inp), nextEdge1(ine1), nextEdge2(ine2), name(iname) {}
    glm::vec2 position;

    glm::vec2 nextEdge1;
    glm::vec2 nextEdge2;

    std::string name; //only debug
    bool operator< (const EnhancedPoint & ep) const {
        float firstAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(position - theTouchInputManager.getTouchLastPosition()));
        float secondAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(ep.position - theTouchInputManager.getTouchLastPosition()));
        return (firstAngle < secondAngle);
    }
    bool operator== (const EnhancedPoint & ep) const {
        return (glm::length2(position - ep.position) < 0.0001f);
    }
    bool operator== (const glm::vec2 & pos) const {
        return (glm::length2(position - pos) < 0.0001f);
    }
    bool operator== (const std::string & inName) const {
        return (name == inName);
    }
};

inline std::ostream & operator<<(std::ostream & o, const EnhancedPoint & ep) {
    o << "name='" << ep.name << "': position='" << ep.position << "' nextEdge1='" << ep.nextEdge1 << "', nextEdge2='" << ep.nextEdge2 << "'";
    return o;
}

// Return minimum distance between line segment vw and point p
// see http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
float distancePointToSegment(const glm::vec2 & v, const glm::vec2 & w, const glm::vec2 & p, glm::vec2 * projectionResult = 0) {
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
    if (projectionResult) {
        *projectionResult = projectionOnSegment;
    }
    //drawPoint(projectionOnSegment, Color(1., 0., 0.));

    return glm::length(projectionOnSegment - p);
}

//project line P on line Q and save the intersection point
bool getProjection(const glm::vec2 & pA, const glm::vec2 & pB, const glm::vec2 & qA, const glm::vec2 & qB, glm::vec2 * intersectionPoint = 0) {
    float denom = ((qB.y - qA.y)*(pB.x - pA.x)) -
                      ((qB.x - qA.x)*(pB.y - pA.y));

    float nume_a = ((qB.x - qA.x)*(pA.y - qA.y)) -
                   ((qB.y - qA.y)*(pA.x - qA.x));

    float nume_b = ((pB.x - pA.x)*(pA.y - qA.y)) -
                   ((pB.y - pA.y)*(pA.x - qA.x));

    if(denom == 0.0f)
    {
        return false;
    }

    float ua = nume_a / denom;
    float ub = nume_b / denom;

    const float eps = 0.0001;

    if(ub > - eps && ub <= 1.0f + eps)
    {
        if (intersectionPoint) {
            // Get the intersection point.
            intersectionPoint->x = pA.x + ua*(pB.x - pA.x);
            intersectionPoint->y = pA.y + ua*(pB.y - pA.y);
        }
        return true;
    }

    return false;
}


// var endpoints;      # list of endpoints, sorted by angle
// var open = [];      # list of walls, sorted by distance

// loop over endpoints:
//     remember which wall is nearest
//     add any walls that BEGIN at this endpoint to 'walls'
//     remove any walls that END at this endpoint from 'walls'

//     SORT the open list

//     if the nearest wall changed:
//         fill the current triangle and begin a new one
void BlockSystem::DoUpdate(float) {
    currentDrawPointIndice = 0;
    currentDrawEdgeIndice = 0;
    currentDrawTriangleColorIndice = 0;
    LOGI("\n");

    const glm::vec2 pointOfView = theTouchInputManager.getTouchLastPosition();
    drawPoint(pointOfView, "pointOfView", Color(1., .8, 0));
    std::list<EnhancedPoint> points;

    float sx = PlacementHelper::ScreenWidth / 2.;
    float sy = PlacementHelper::ScreenHeight / 2.;

    //add the walls
    glm::vec2 externalWalls[4] = {
        glm::vec2(-sx, sy), // top left
        glm::vec2(sx, sy), // top right
        glm::vec2(sx, -sy), // bottom right
        glm::vec2(-sx, -sy), // bottom left
    };
    points.push_back(EnhancedPoint(externalWalls[0], externalWalls[1], externalWalls[1], "wall top left"));
    points.push_back(EnhancedPoint(externalWalls[1], externalWalls[2], externalWalls[2], "wall top right"));
    points.push_back(EnhancedPoint(externalWalls[2], externalWalls[3], externalWalls[3], "wall bottom right"));
    points.push_back(EnhancedPoint(externalWalls[3], externalWalls[0], externalWalls[0], "wall bottom left"));

    FOR_EACH_ENTITY(Block, e)
        TransformationComponent * tc = TRANSFORM(e);

        // warning: rotation not handled yet
        glm::vec2 rectanglePoints[4] = {
            tc->position - tc->size / 2.f, //bottom left
            tc->position + glm::vec2(tc->size.x, -tc->size.y) / 2.f, //bottom right
            tc->position + tc->size / 2.f, //top right
            tc->position - glm::vec2(tc->size.x, -tc->size.y) / 2.f, //top left
        };
        points.push_back(EnhancedPoint(rectanglePoints[3], rectanglePoints[2], rectanglePoints[0],
            theEntityManager.entityName(e) + "- top left"));
        points.push_back(EnhancedPoint(rectanglePoints[2], rectanglePoints[1], rectanglePoints[3],
            theEntityManager.entityName(e) + "- top right"));
        points.push_back(EnhancedPoint(rectanglePoints[1], rectanglePoints[0], rectanglePoints[2],
            theEntityManager.entityName(e) + "- bottom right"));
        points.push_back(EnhancedPoint(rectanglePoints[0], rectanglePoints[3], rectanglePoints[1],
            theEntityManager.entityName(e) + "- bottom left"));
    }

    std::list<std::pair<glm::vec2, glm::vec2>> walls;
    for (auto point : points) {
        if ( glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(point.position)) >
            glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(point.nextEdge1)))
            walls.push_back(std::make_pair( point.position, point.nextEdge1 ));
        else
            walls.push_back(std::make_pair( point.nextEdge1, point.position ));

        //il FAUT le 2eme, parce que la caméra peut voir le 1er ou le 2nd selon où elle est
        if (point.nextEdge1 != point.nextEdge2) {
            if ( glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(point.position)) >
                glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(point.nextEdge2)))
                walls.push_back(std::make_pair( point.position, point.nextEdge2 ));
            else
                walls.push_back(std::make_pair( point.nextEdge2, point.position ));
        }
    }
    walls.sort([pointOfView] (std::pair<glm::vec2, glm::vec2> & w1, std::pair<glm::vec2, glm::vec2> & w2) {
        float firstDist = distancePointToSegment(w1.first, w1.second, pointOfView);
        float secondDist = distancePointToSegment(w2.first, w2.second, pointOfView);

        //if the points are the same, compare the distance from the 2 extremity
        if (firstDist - secondDist < 0.001f) {
            return glm::length2(pointOfView - w1.first) + glm::length2(pointOfView - w1.second) <
                glm::length2(pointOfView - w2.first) + glm::length2(pointOfView - w2.second);
        }

        return firstDist < secondDist;
    });

    int w = 0;
    for (auto wall : walls) {
        LOGI(++w << ". " << wall.first << " <-> " << wall.second);
    }

    //order in clockwise mode
    points.sort([] (const EnhancedPoint & ep1, const EnhancedPoint & ep2) {
        return ep2 < ep1;
    });

    glm::vec2 startPoint = points.begin()->position;

    int i = 0;
    for (auto point : points) {
        LOGI(++i << ". " << point.name);
        drawPoint(point.position, point.name);

        glm::vec2 endPoint;

        std::pair<glm::vec2, glm::vec2> activeWall;
        for (auto wall : walls) {
            // getActiveWall(startPoint, point.position);
            // LOGI("\ttrying wall " << wall.first << " <-> " << wall.second);
            //le facteur 100 est juste là pour faire une demi droite "infinie"
            bool firstWall = IntersectionUtil::lineLine(pointOfView, startPoint + 100.f * (startPoint - pointOfView), wall.first, wall.second, 0);
            bool secondWall = IntersectionUtil::lineLine(pointOfView, point.position + 100.f * (point.position - pointOfView), wall.first, wall.second, 0);
            if (firstWall && secondWall) {
                activeWall = wall;
                LOGI("\tActive wall is " << activeWall.first << "<->" << activeWall.second);
                break;
            } else {
                // if (firstWall) LOGI("\tfound first but not the second");
                // if (secondWall) LOGI("\tfound second but not the first");
            }
        }
        LOGF_IF(activeWall.first == glm::vec2(0.f) && activeWall.second == glm::vec2(0.f), "active wall not found");

        if (IntersectionUtil::pointLine(startPoint, activeWall.first, activeWall.second) &&
            (glm::length2(activeWall.second - point.position) < 0.0001f || glm::length2(activeWall.first - point.position) < 0.0001f)) {
             LOGI("\tDid the whole wall in a single time (no obstacle)");
            endPoint = point.position;
        } else {
            LOGI(IntersectionUtil::pointLine(startPoint, activeWall.first, activeWall.second));
            LOGI("\tGoing on a foreign wall! projecting point " << point.position << " into the current active wall..." << activeWall.first << " x " << activeWall.second);
            getProjection(pointOfView, point.position, activeWall.first, activeWall.second, & endPoint);
        }

        drawTriangle(pointOfView, startPoint, endPoint);

        //si on a fini notre mur, le point de départ pour le prochain mur c'est le projeté du point final
        // sur le mur le plus proche
        if (IntersectionUtil::pointLine(startPoint, activeWall.first, activeWall.second)
        && (activeWall.second == point.position)) {
            LOGI("We ended a wall, searching for startPoint of next wall...");
            std::pair<glm::vec2, glm::vec2> nextActiveWall;
            for (auto wall : walls) {
                    LOGI("trying.." << wall.first << "<->" << wall.second);

                //next active wall can't be the current one...
                if ((wall.first == activeWall.first && wall.second == activeWall.second)
                    || (wall.first == activeWall.second && wall.second == activeWall.first))
                    continue;

                //nor end at current point
                if (wall.second != point.position) {
                //if (wall.second != point.position) {
                    if (IntersectionUtil::lineLine(pointOfView, point.position + 100.f * (point.position - pointOfView), wall.first, wall.second, &nextActiveWall.first)) {
                        nextActiveWall.second = wall.second;
                        LOGI("\tNext active wall is " << nextActiveWall.first << "<->" << nextActiveWall.second << " ( " << wall.first << " <-> " << wall.second << " before proj)");
                        break;
                    }
                }
            }

            //means we are on last point
            if (nextActiveWall.first == glm::vec2(0.f) && nextActiveWall.second == glm::vec2(0.f)) {
                LOGI( "next active wall not found, meaning we loop is ended");
                nextActiveWall.first = externalWalls[3];
                nextActiveWall.second = externalWalls[0];
            }

            if (! getProjection(pointOfView, point.position, nextActiveWall.first, nextActiveWall.second, & startPoint)) {
                LOGF("could not project!!");
            }
        //si le point qu'on est actuellement dessus est DERRIERE le mur actuellement
        // actif, le startPoint doit rester sur le mur actif
        } else {
            glm::vec2 intersection;
            bool isIntercepting = IntersectionUtil::lineLine(pointOfView, point.position, activeWall.first, activeWall.second, &intersection);
            if (isIntercepting && glm::length2(intersection - point.position) > 0.0001f) {
                LOGI(intersection << " != " << point.position);
                LOGI("\tCurrent point is behind the activeWall, starting point is activeWall.first");
                startPoint = activeWall.first;
            //sinon
            } else {
                //sinon c'est simplement le point courant
                LOGI("Starting point is now this point");
                startPoint = point.position;
            }
        }
        LOGI("\tNext startPoint is " << startPoint);

    }
    // on affiche le dernier triangle
    drawTriangle(pointOfView, startPoint, points.front().position);

    //hide the extra
    while (currentDrawPointIndice < (int)drawPointList.size()) {
        RENDERING(drawPointList[currentDrawPointIndice])->show = false;
        ++currentDrawPointIndice;
    }
    while (currentDrawEdgeIndice < (int)drawEdgeList.size()) {
        RENDERING(drawEdgeList[currentDrawEdgeIndice])->show = false;
        ++currentDrawEdgeIndice;
    }
}

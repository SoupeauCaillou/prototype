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
Entity drawPoint(const glm::vec2& position, const Color & color = Color(0.5, 0.5, 0.5)) {
    Entity vector;

    if (currentDrawPointIndice == (int)drawPointList.size()) {
        vector = theEntityManager.CreateEntity("vector");
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

    if(ub >= 0.0f && ub <= 1.0f)
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
    drawPoint(pointOfView, Color(1., .8, 0));
    std::list<EnhancedPoint> points;

    float sx = PlacementHelper::ScreenWidth / 2.;
    float sy = PlacementHelper::ScreenHeight / 2.;

    //add the walls
    glm::vec2 externalWalls[5] = { glm::vec2(-sx, pointOfView.y), glm::vec2(-sx, -sy), glm::vec2(sx, -sy), glm::vec2(sx, sy), glm::vec2(-sx, sy) };
    points.push_back(EnhancedPoint(externalWalls[0], externalWalls[4], externalWalls[4], "wall middle left"));
    points.push_back(EnhancedPoint(externalWalls[1], externalWalls[2], externalWalls[0], "wall bottom left"));
    points.push_back(EnhancedPoint(externalWalls[2], externalWalls[3], externalWalls[1], "wall bottom right"));
    points.push_back(EnhancedPoint(externalWalls[3], externalWalls[4], externalWalls[2], "wall top right"));
    points.push_back(EnhancedPoint(externalWalls[4], externalWalls[0], externalWalls[3], "wall top left"));

    FOR_EACH_ENTITY(Block, e)
        TransformationComponent * tc = TRANSFORM(e);

        // warning: rotation not handled yet
        glm::vec2 rectanglePoints[4] = {
            tc->position - tc->size / 2.f, //bottom left
            tc->position + glm::vec2(tc->size.x, -tc->size.y) / 2.f, //bottom right
            tc->position + tc->size / 2.f, //top right
            tc->position - glm::vec2(tc->size.x, -tc->size.y) / 2.f, //top left
        };
        points.push_back(EnhancedPoint(rectanglePoints[0], rectanglePoints[3], rectanglePoints[1], theEntityManager.entityName(e) + "- top left"));
        points.push_back(EnhancedPoint(rectanglePoints[1], rectanglePoints[0], rectanglePoints[2], theEntityManager.entityName(e) + "- top right"));
        points.push_back(EnhancedPoint(rectanglePoints[2], rectanglePoints[1], rectanglePoints[3], theEntityManager.entityName(e) + "- bottom left"));
        points.push_back(EnhancedPoint(rectanglePoints[3], rectanglePoints[2], rectanglePoints[0], theEntityManager.entityName(e) + "- bottom right"));
    }

    //order in clockwise mode
    points.sort([] (const EnhancedPoint & ep1, const EnhancedPoint & ep2) {
        return ep2 < ep1;
    });

    //add first point
    float minWallNorm = 10000.f;
    EnhancedPoint minWallStartPoint;
    glm::vec2 endPointPosition;

    for (auto & point : points) {
        std::list<glm::vec2> pos;
        pos.push_back(point.nextEdge1);
        pos.push_back(point.nextEdge2);
        for (auto endpoint : pos) {
            float wallNorm = distancePointToSegment(point.position, endpoint, pointOfView);

            glm::vec2 projection(-100.f);

            if (( wallNorm < minWallNorm )
            && (IntersectionUtil::lineLine(pointOfView, glm::vec2(- PlacementHelper::ScreenWidth,
                pointOfView.y), point.position, endpoint, &projection))) {

                minWallNorm = wallNorm;
                minWallStartPoint.position = projection;
                minWallStartPoint.nextEdge1 = point.position;
                minWallStartPoint.nextEdge2 = point.position;
                endPointPosition = endpoint;
                minWallStartPoint.name = "start point";

                LOGI("new min wall: " << point.name << ": " << point.position << " <-> " << endpoint);
                LOGI("projection gave " << projection);

            }
        }
    }

    if (minWallNorm < glm::abs(pointOfView.x + sx) && minWallNorm < glm::abs(pointOfView.y + sy)) {
        LOGI("nearest wall is not an external wall (there is a block somewhere right?" << minWallNorm <<" "<<
            glm::abs(pointOfView.x + sx) <<" "<< glm::abs(pointOfView.y + sy));

        //recalculate the projection of externalWall[0] because it is no more alligned with the pov y
        auto middleLeftExternalWallPoint = std::find(points.begin(), points.end(), "wall middle left");

        glm::vec2 oldPosition = middleLeftExternalWallPoint->position;

        getProjection(pointOfView, externalWalls[1], externalWalls[4], middleLeftExternalWallPoint->position);

        for (auto & point : points) {
            if (point.nextEdge1 == oldPosition)
                point.nextEdge1 = middleLeftExternalWallPoint->position;
            if (point.nextEdge2 == oldPosition)
                point.nextEdge2 = middleLeftExternalWallPoint->position;

            if (point.position == minWallStartPoint.nextEdge1) {
                auto neighboors = &point;

                do {
                    neighboors->nextEdge2 = neighboors->nextEdge1;
                    auto it = std::find(points.begin(), points.end(), neighboors->nextEdge1);
                    neighboors = &(*it);
                } while (! (*neighboors == point));
            }
            if (point.position == endPointPosition  ) {
                if (point.nextEdge1 == minWallStartPoint.nextEdge1) {
                    point.nextEdge1 = minWallStartPoint.position;
                }
                if (point.nextEdge2 == minWallStartPoint.nextEdge1) {
                    point.nextEdge2 = minWallStartPoint.position;
                }
            } else if (point.position == minWallStartPoint.nextEdge1  ) {
                if (point.nextEdge2 == endPointPosition) {
                    point.nextEdge2 = minWallStartPoint.position;
                }
                if (point.nextEdge1 == endPointPosition) {
                    point.nextEdge1 = minWallStartPoint.position;
                }
            }
        }
        points.push_front(minWallStartPoint);
    }

    LOGI( "nearest wall is: " << minWallStartPoint.nextEdge1 << " <->" << endPointPosition);
    LOGI( "Start point will be: " << minWallStartPoint.position);
    drawPoint(minWallStartPoint.position, Color(.8f, .5f, 0.f));
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


    std::list<std::pair<glm::vec2, glm::vec2>> walls;

    std::list<EnhancedPoint> alreadyPassedPoints;

    std::pair<glm::vec2, glm::vec2> nearestWall;
    bool hasNearestWall = false;

    int i = 0;
    for (auto pointIt = points.begin(); pointIt != points.end(); ++pointIt) {
        auto point = *pointIt;

        drawPoint(point.position);
        LOGI(++i << ". '" << point.name << "' " << glm::degrees(glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(point.position)))
            << " " << point.position);

        // Sauvegarde du mur le plus proche
        hasNearestWall =  (walls.size() > 0);
        if (hasNearestWall) {
            nearestWall = walls.front();
            LOGI("\tNearest wall is " << nearestWall.first << " <-> " << nearestWall.second);
        } else {
            LOGI("\tNo nearest wall");
        }

        // On supprime tous les segments qui ont comme fin le point qu'on va rajouter (sinon ils y seront 2 fois)
        for (auto wall = walls.begin(); wall != walls.end();) {
            auto it = wall;
            //in case of deletion, avoid problem with iterators
            ++wall;
            LOGI("\t\t\tWall distance: " << distancePointToSegment(pointOfView, it->first, it->second, 0) << " for " << it->first << " <-> " << it->second);

            if (it->second == point.position) {
                LOGI("\t\tRemoving point " << it->second << " as end position - erasing it");
                walls.erase(it);
            }
        }

        if (std::find(points.begin(), pointIt, point.nextEdge1) == pointIt) {
            // LOGI("\t\tAdding1 " << point.position << " <-> " << point.nextEdge1);
            // walls.push_back(std::make_pair(point.position, point.nextEdge1));
        }
        if (/*point.nextEdge2 != point.nextEdge1 && */std::find(points.begin(), pointIt, point.nextEdge2) == pointIt) {
            LOGI("\t\tAdding2 " << point.position << " <-> " << point.nextEdge2);
            walls.push_back(std::make_pair(point.position, point.nextEdge2));
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

        auto it = walls.begin();
        for (unsigned i = 0; i < walls.size(); ++i) {
            LOGI("\t\t" << i << ": " << distancePointToSegment(it->first, it->second, pointOfView) << ":" << it->first << " x " << it->second);
            ++it;
        }
        // Si on a changé de segment le plus proche, on plot un triangle
        // OU si on a atteint le bout du mur le plus proche
        if (hasNearestWall) {
            bool newNearestWall = walls.size() > 0 && ((glm::length2(walls.front().first - nearestWall.first) > 0.00001f &&
            glm::length2(walls.front().second - nearestWall.second)> 0.0001f));
            bool nearestWallEndPointReached = (glm::length2(nearestWall.second - point.position) < 0.0001f);

            bool nearestWallEndPointAlreadyVisited = false;
            for (auto it : alreadyPassedPoints) {
                if (glm::length2(it.position - nearestWall.second) < 0.0001f) {
                    nearestWallEndPointAlreadyVisited = true;
                    break;
                }
            }
            if (newNearestWall) {
                LOGI("New nearest wall! " << walls.front().first << " <-> " << walls.front().second << ": "
                    << distancePointToSegment(walls.front().first, walls.front().second, pointOfView) <<
                    " < " << distancePointToSegment(nearestWall.first, nearestWall.second, pointOfView) <<  ". Drawing triangle");
            } else if (nearestWallEndPointReached) {
                LOGI("End of nearest wall reached! " << nearestWall.first << " <-> " << nearestWall.second << ". Drawing triangle");
            } else if (nearestWallEndPointAlreadyVisited) {
                LOGF("End of nearest wall had already been visited! " << nearestWall.first << " <-> " << nearestWall.second << ". Drawing triangle");
            }

            glm::vec2 realPointEnd = point.position;
            getProjection(pointOfView, point.position, nearestWall.first, nearestWall.second, &realPointEnd);


            if (newNearestWall || nearestWallEndPointReached || nearestWallEndPointAlreadyVisited) {
                drawTriangle(pointOfView, nearestWall.first, realPointEnd);
                //update nearestwall since it has changed
                if (walls.size() > 0) {
                    nearestWall = walls.front();

                    getProjection(pointOfView, point.position, walls.front().first, walls.front().second, &walls.front().first);
                } else {
                    LOGI(points.front());
                    nearestWall.second = externalWalls[0];
                    nearestWall.first = externalWalls[4];
                    if (point.position != externalWalls[4]) {
                        if (! getProjection(pointOfView, point.position, externalWalls[1], externalWalls[4], &nearestWall.first))
                            LOGI("could not project!!!!!!!");
                        LOGI(nearestWall.first);
                    }
                }
            }
        }
        alreadyPassedPoints.push_back(point);
    }


    LOGI("Last nearest: " << nearestWall.first << "<->" << nearestWall.second << " and first point was " << points.front().position);
    drawTriangle(pointOfView, nearestWall.first, nearestWall.second);

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

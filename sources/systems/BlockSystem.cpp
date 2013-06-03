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

//activate or not logs (debug)
#ifdef SAC_DEBUG
static bool debugBlockSystem = !true;
#else
static bool debugBlockSystem = false;
#endif

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
    LOGI_IF(debugBlockSystem, "\tPlotting triangle: '" << pointOfView << "' x '" << first << "' x '" << second << "'");

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
        const glm::vec2 touchLastPosition = theTouchInputManager.getTouchLastPosition();
        float firstAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(position - touchLastPosition));
        float secondAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(ep.position - touchLastPosition));
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
inline std::ostream & operator<<(std::ostream & o, const std::pair<glm::vec2, glm::vec2> & wall) {
    o << wall.first << " <-> " << wall.second;
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

// retourne le mur actif entre les 2 points, vus de la caméra
std::pair<glm::vec2, glm::vec2> getActiveWall(const std::list<std::pair<glm::vec2, glm::vec2>> & walls,
    const glm::vec2 & pointOfView, const glm::vec2 & firstPoint, const glm::vec2 & secondPoint) {

    float nearestWallDistance = 100000.f;
    std::pair<glm::vec2, glm::vec2> nearestWall;

    for (auto wall : walls) {
        // LOGI_IF(debugBlockSystem, "\ttrying wall " << wall.first << " <-> " << wall.second);
        glm::vec2 firstIntersectionPoint, secondIntersectionPoint;

        //le facteur 100 est juste là pour faire une demi droite "infinie"
        bool wallContainsFirstPoint = IntersectionUtil::lineLine(pointOfView, firstPoint + 100.f * (firstPoint - pointOfView), wall.first, wall.second, &firstIntersectionPoint);
        bool wallContainsSecondPoint = IntersectionUtil::lineLine(pointOfView, secondPoint + 100.f * (secondPoint - pointOfView), wall.first, wall.second, &secondIntersectionPoint);

        // LOGI_IF(debugBlockSystem, "test current wall: " << wall << " " << wallContainsFirstPoint << "( " << firstPoint << " ) | " << wallContainsSecondPoint << " ( " << secondPoint << " ) " );
        if (wallContainsFirstPoint && wallContainsSecondPoint) {
            // bool isNearest = false;
            float minDist = glm::min(glm::length2(firstIntersectionPoint - pointOfView), glm::length2(secondIntersectionPoint - pointOfView));

            LOGI_IF(debugBlockSystem, "found a candidate wall: " << wall << " for distance: " << minDist
                << " points: " << firstIntersectionPoint << " and " << secondIntersectionPoint);

            if (minDist < nearestWallDistance - 0.00001f) {
                LOGI_IF(debugBlockSystem, "found a new nearest wall: " << wall << " for distance: " << minDist << " < " << nearestWallDistance);

                nearestWallDistance = minDist;
                nearestWall = wall;
            }
        } else {
            if (wallContainsFirstPoint) LOGI_IF(debugBlockSystem, "\tfound first but not the second");
            if (wallContainsSecondPoint) LOGI_IF(debugBlockSystem, "\tfound second but not the first");
        }
    }
    LOGF_IF(debugBlockSystem && nearestWallDistance == 100000.f, "Couldn't find a wall between points " << firstPoint << " and " << secondPoint);

    LOGI_IF(debugBlockSystem, "\tActive wall is " << nearestWall);
    return nearestWall;
}

void insertInWallsIfNotPresent(std::list<std::pair<glm::vec2, glm::vec2>> & walls, const glm::vec2 & pointOfView, const glm::vec2 & firstPoint,  const glm::vec2 & secondPoint) {

    float firstAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(firstPoint - pointOfView ));
    float secondAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(secondPoint - pointOfView ));

    // le premier point c'est celui avec le plus grand angle (premier où la caméra passera)
    auto pair = (firstAngle > secondAngle) ? std::make_pair( firstPoint, secondPoint ) : std::make_pair( secondPoint, firstPoint );

    //s'il n'est pas déjà présent, on l'insère dans la liste
    if (std::find(walls.begin(), walls.end(), pair) == walls.end()) {
        walls.push_back(pair);
    }
}

void BlockSystem::DoUpdate(float) {
    currentDrawPointIndice = 0;
    currentDrawEdgeIndice = 0;
    currentDrawTriangleColorIndice = 0;
    LOGI_IF(debugBlockSystem, "\n");

    const glm::vec2 pointOfView = theTouchInputManager.getTouchLastPosition();
    drawPoint(pointOfView, "pointOfView", Color(1., .8, 0));
    std::list<EnhancedPoint> points;

    //Première étape : on ajoute les points des blocks
    FOR_EACH_ENTITY(Block, e)
        TransformationComponent * tc = TRANSFORM(e);

        glm::vec2 offset = glm::rotate(tc->size / 2.f, tc->rotation);

        glm::vec2 rectanglePoints[4] = {
            tc->position - offset, //bottom left
            // tc->position + glm::vec2(offset.x, -offset.y) / 2.f, //bottom right
            tc->position + offset, //top right
            // tc->position - glm::vec2(offset.x, -offset.y) / 2.f, //top left
        };
        points.push_back(EnhancedPoint(rectanglePoints[0], rectanglePoints[1], rectanglePoints[1],
            theEntityManager.entityName(e) + "- first point"));
        points.push_back(EnhancedPoint(rectanglePoints[1], rectanglePoints[0], rectanglePoints[0],
            theEntityManager.entityName(e) + "- second point"));

        // points.push_back(EnhancedPoint(rectanglePoints[3], rectanglePoints[2], rectanglePoints[0],
        //     theEntityManager.entityName(e) + "- top left"));
        // points.push_back(EnhancedPoint(rectanglePoints[2], rectanglePoints[1], rectanglePoints[3],
        //     theEntityManager.entityName(e) + "- top right"));
        // points.push_back(EnhancedPoint(rectanglePoints[1], rectanglePoints[0], rectanglePoints[2],
        //     theEntityManager.entityName(e) + "- bottom right"));
        // points.push_back(EnhancedPoint(rectanglePoints[0], rectanglePoints[3], rectanglePoints[1],
        //     theEntityManager.entityName(e) + "- bottom left"));
    }

    // et les points des murs extérieurs
    float sx = PlacementHelper::ScreenWidth / 2.;
    float sy = PlacementHelper::ScreenHeight / 2.;

    glm::vec2 externalWalls[5] = {
        glm::vec2(-sx, pointOfView.y), // middle left (FIRST POINT)
        glm::vec2(-sx, sy), // top left
        glm::vec2(sx, sy), // top right
        glm::vec2(sx, -sy), // bottom right
        glm::vec2(-sx, -sy), // bottom left
    };
    points.push_back(EnhancedPoint(externalWalls[1], externalWalls[2], externalWalls[2], "wall top left"));
    points.push_back(EnhancedPoint(externalWalls[2], externalWalls[3], externalWalls[3], "top right"));
    points.push_back(EnhancedPoint(externalWalls[3], externalWalls[4], externalWalls[4], "wall bottom right"));
    points.push_back(EnhancedPoint(externalWalls[4], externalWalls[0], externalWalls[0], "wall bottom left"));

    // on trie les points par angle, en sens horaire (min = max = (-1, 0))
    points.sort([] (const EnhancedPoint & ep1, const EnhancedPoint & ep2) {
        return ep2 < ep1;
    });

    //on s'assure que le 1er point qui sera parcouru est le "wall middle left"
    points.push_front(EnhancedPoint(externalWalls[0], externalWalls[1], externalWalls[1], "wall middle left"));

    //on garde la liste de tous les murs disponibles, le 1er point est le premier point que la caméra rencontrera
    std::list<std::pair<glm::vec2, glm::vec2>> walls;
    for (auto point : points) {
        insertInWallsIfNotPresent(walls, pointOfView, point.position, point.nextEdge1);

        //il FAUT le 2eme, parce que la caméra peut voir le 1er ou le 2nd selon où elle est
        if (point.nextEdge1 != point.nextEdge2) {
            insertInWallsIfNotPresent(walls, pointOfView, point.position, point.nextEdge2);
        }
    }
    // on trie les murs par distance à la caméra, du plus proche au plus lointain
    walls.sort([pointOfView] (std::pair<glm::vec2, glm::vec2> & w1, std::pair<glm::vec2, glm::vec2> & w2) {
        float firstDist = distancePointToSegment(w1.first, w1.second, pointOfView);
        float secondDist = distancePointToSegment(w2.first, w2.second, pointOfView);

        return firstDist < secondDist;
    });

    // du debug
    FOR_EACH_ENTITY(Block, e)
        if (IntersectionUtil::pointRectangle(pointOfView, TRANSFORM(e)->position, TRANSFORM(e)->size)) {
            LOGI_IF(debugBlockSystem, "Point of view is INSIDE the block " << theEntityManager.entityName(e));
        }
    }
    int w = 0;
    for (auto wall : walls) {
        LOGI_IF(debugBlockSystem, ++w << ". " << wall);
    }

    int i = 0;

    // le point de départ de l'éclairage

    LOGI_IF(debugBlockSystem, ++i << ". " << points.begin()->name << " (angle="
        << glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(points.begin()->position - pointOfView))
        << ", position = " << points.begin()->position << ")");
    glm::vec2 startPoint = points.begin()->position;
    auto activeWall = getActiveWall(walls, pointOfView, startPoint, glm::vec2(-sx, pointOfView.y));
    if (! IntersectionUtil::pointLine(startPoint, activeWall.first, activeWall.second)) {
        LOGI_IF(debugBlockSystem, "First point ( " << startPoint << " ) is NOT on the active wall. There must be a block between it and the camera right? So we'll project\
            that point on the active wall and take this point as the start point...");
        if (! getProjection(pointOfView, startPoint, activeWall.first, activeWall.second, & startPoint))
            LOGF("could not project!!");
    }
    LOGI_IF(debugBlockSystem, "Start point is " << startPoint);
#if SAC_DEBUG
    drawPoint(points.begin()->position, points.begin()->name);
#endif
    //le dernier point de l'éclérage
    glm::vec2 endPoint;

    // on commence directement au 2eme du coup vu qu'on a déjà pris le 1er au dessus
    for (auto pointIt = ++points.begin(); pointIt != points.end(); ++pointIt) {
        auto point = *pointIt;

        LOGI_IF(debugBlockSystem, ++i << ". " << point.name << " (angle=" <<
            glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(point.position - pointOfView)) << ", position = " << point.position <<  ")");
#if SAC_DEBUG
        drawPoint(point.position, point.name);
#endif



        // on cherche le mur actif (c'est à dire le mur le plus proche qui contient le startPoint ET le point actuel)
        activeWall = getActiveWall(walls, pointOfView, startPoint, point.position);

        //si on a pas trouvé de mur actif il y a un bug quelque part ...
        LOGF_IF(activeWall.first == glm::vec2(0.f) && activeWall.second == glm::vec2(0.f), "active wall not found");


        bool isFirstPointOnWall = IntersectionUtil::pointLine(startPoint, activeWall.first, activeWall.second);
        bool isCurrentPointTheEndPoint = IntersectionUtil::pointLine(point.position, activeWall.first, activeWall.second);
        bool hasEndedCurrentActiveWall = isFirstPointOnWall && isCurrentPointTheEndPoint;

        //si on a terminé un mur, le point final à éclairer c'est juste notre point
        if (hasEndedCurrentActiveWall) {
            endPoint = point.position;
        //sinon, ça veut dire que le point courant et le startpoint ne sont plus sur le même mur: changement de mur.
        //la zone a eclairé c'est donc le projeté du point courant sur l'ancien mur actif
        } else {
            if (isFirstPointOnWall) LOGI_IF(debugBlockSystem, "\t\tfirst cond: true");
            else LOGI_IF(debugBlockSystem, "\t\tfirst cond: false -> " << startPoint << " not on " << activeWall);

            if (isCurrentPointTheEndPoint) LOGI_IF(debugBlockSystem, "\t\tsecond cond: true");
            else LOGI_IF(debugBlockSystem, "\t\tsecond cond: false -> " << activeWall.second << " != " << point.position);

            LOGI_IF(debugBlockSystem, "\tCurrent point is not on the active wall! Projecting point " << point.position << " into the old active wall..." << activeWall);
            getProjection(pointOfView, point.position, activeWall.first, activeWall.second, & endPoint);
        }

        //finalement on affiche notre zone à éclairer
        drawTriangle(pointOfView, startPoint, endPoint);


        // maintenant qu'on a fini le mur, il faut chercher le futur mur actif, et projeter notre point dessus
        if (hasEndedCurrentActiveWall) {
            LOGI_IF(debugBlockSystem, "\tWe ended a wall, searching for startPoint of next wall...");
            auto nextPointIt = pointIt;
            ++nextPointIt;

            std::pair<glm::vec2, glm::vec2> nextActiveWall;
            if (nextPointIt != points.end()) {
                glm::vec2 nextPoint = (pointIt == points.end() ? points.front().position : nextPointIt->position);
                nextActiveWall = getActiveWall(walls, pointOfView, point.position, nextPoint);
            }

            // si on est sur le point en bas à gauche du mur extérieur, il n'y a pas de mur actif après (même si y a d'autres blocks)
            // on garde le point comme startPoint
            if (nextActiveWall.first == glm::vec2(0.f) && nextActiveWall.second == glm::vec2(0.f)) {
                LOGI_IF(debugBlockSystem,  "\tNext active wall not found, meaning we are at bottom left external wall position");
                startPoint = point.position;
            } else {
                // sinon si on arrive pas à projeter sur le mur suivant, il y a un problème
                if (! getProjection(pointOfView, point.position, nextActiveWall.first, nextActiveWall.second, & startPoint)) {
                    LOGF("could not project!!");
                }
            }

        //il faut regarder si le point courant est visible ou pas. S'il est derrière le mur actif, le point de départ reste sur le mur actif
        } else {
            glm::vec2 intersection;
            bool isIntercepting = IntersectionUtil::lineLine(pointOfView, point.position, activeWall.first, activeWall.second, &intersection);
            if (isIntercepting && glm::length2(intersection - point.position) > 0.0001f) {
                // LOGI_IF(debugBlockSystem, intersection << " != " << point.position);
                LOGI_IF(debugBlockSystem, "\tCurrent point is behind the activeWall, starting point is the projection point on the active wall");
                startPoint = intersection;
            //sinon il est la base du nouveau mur actif, donc on met à jour le ponit de départ
            } else {
                LOGI_IF(debugBlockSystem, "\tStarting point is now this point for next active wall");
                startPoint = point.position;
            }
        }
        LOGI_IF(debugBlockSystem, "\tNext startPoint is " << startPoint);
    }

    // 2 cas pour le dernier point:
    // 1) soit le dernier point et le 1er point sont un seul mur, et dans ce cas on affiche le mur (mur extérieur par ex)
    // 2) soit ils sont sur 2 murs distincts, et dans ce cas on projete les 2 points sur le mur actif (2 blocks, 1 de chaque côté de l'axe des abcisses)
    activeWall = getActiveWall(walls, pointOfView, startPoint, points.front().position);
    LOGI_IF(debugBlockSystem, "\tLast activeWall is " << activeWall << " so projeting " << startPoint << " and " << points.front().position << " on it.");
    getProjection(pointOfView, startPoint, activeWall.first, activeWall.second, & startPoint);
    getProjection(pointOfView, points.front().position, activeWall.first, activeWall.second, & endPoint);

    drawTriangle(pointOfView, startPoint, endPoint);

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


void BlockSystem::CleanEntities() {
    while (drawPointList.begin() != drawPointList.end()) {
        theEntityManager.DeleteEntity(*--drawPointList.end());
        drawPointList.pop_back();
    }
    while (drawEdgeList.begin() != drawEdgeList.end()) {
        theEntityManager.DeleteEntity(*--drawEdgeList.end());
        drawEdgeList.pop_back();
    }
}

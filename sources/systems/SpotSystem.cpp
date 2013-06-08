#include "SpotSystem.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/BlockSystem.h"
#include "systems/ButtonSystem.h"

#include "util/drawVector.h"
#include "util/IntersectionUtil.h"

#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/projection.hpp>

#include <algorithm>
#include <ostream>

//activate or not logs (debug)
#ifdef SAC_DEBUG
static bool debugSpotSystem = !true;
#else
static bool debugSpotSystem = false;
#endif

INSTANCE_IMPL(SpotSystem);

SpotSystem::SpotSystem() : ComponentSystemImpl <SpotComponent>("Spot") {
}

static int currentDrawPointIndice = 0;
static std::vector<Entity> drawPointList;
Entity drawPoint(const glm::vec2& position, const std::string name = "point", const Color & color = Color(0.5, 0.5, 0.5)) {
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
static std::vector<std::pair<Entity, Color>> drawTriangleList;
void drawTriangle(const glm::vec2& pointOfView, const glm::vec2& first, const glm::vec2& second) {
    LOGI_IF(debugSpotSystem, "\tPlotting triangle: '" << pointOfView << "' x '" << first << "' x '" << second << "'");

    if (currentDrawTriangleColorIndice == (int)drawTriangleList.size()) {
        Entity e = theEntityManager.CreateEntity("triangle");
        Color c = Color::random();
        c.a = .3;
        ADD_COMPONENT(e, Transformation);
        ADD_COMPONENT(e, Rendering);
        RENDERING(e)->shape = Shape::Triangle;
        RENDERING(e)->color = c;
        RENDERING(e)->dynamicVertices = currentDrawTriangleColorIndice;

        drawTriangleList.push_back(std::make_pair(e, c));
    }
    std::vector<glm::vec2> vert;
    vert.push_back(pointOfView);
    vert.push_back(first);
    vert.push_back(second);
    theRenderingSystem.defineDynamicVertices(currentDrawTriangleColorIndice, vert);
    RENDERING(drawTriangleList[currentDrawTriangleColorIndice].first)->show = true;

    ++currentDrawTriangleColorIndice;
}

inline std::ostream & operator<<(std::ostream & o, const EnhancedPoint & ep) {
    o << "name='" << ep.name << "': position='" << ep.position;
    int i = 0;
    for (auto item : ep.nextEdges)
        o << "' nextEdge" <<  ++i << "='" << item << ", ";
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
    if (norm2 < eps) {
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

// retourne le mur actif entre les 2 points, vus de la caméra
std::pair<glm::vec2, glm::vec2> getActiveWall(const std::list<std::pair<glm::vec2, glm::vec2>> & walls,
    const glm::vec2 & pointOfView, const glm::vec2 & firstPoint, const glm::vec2 & secondPoint) {

    float nearestWallDistance = 100000.f;
    std::pair<glm::vec2, glm::vec2> nearestWall;

    for (auto wall : walls) {
        // LOGI_IF(debugSpotSystem, "\ttrying wall " << wall.first << " <-> " << wall.second);
        glm::vec2 firstIntersectionPoint, secondIntersectionPoint;

        //le facteur 100 est juste là pour faire une demi droite "infinie"
        bool wallContainsFirstPoint = IntersectionUtil::lineLine(pointOfView, firstPoint + 100.f * (firstPoint - pointOfView), wall.first, wall.second, &firstIntersectionPoint);
        bool wallContainsSecondPoint = IntersectionUtil::lineLine(pointOfView, secondPoint + 100.f * (secondPoint - pointOfView), wall.first, wall.second, &secondIntersectionPoint);

        // LOGI_IF(debugSpotSystem, "test current wall: " << wall << " " << wallContainsFirstPoint << "( " << firstPoint << " ) | " << wallContainsSecondPoint << " ( " << secondPoint << " ) " );
        if (wallContainsFirstPoint && wallContainsSecondPoint) {
            // bool isNearest = false;
            float minDist = glm::min(glm::length2(firstIntersectionPoint - pointOfView), glm::length2(secondIntersectionPoint - pointOfView));

            LOGI_IF(debugSpotSystem, "found a candidate wall: " << wall << " for distance: " << minDist
                << " points: " << firstIntersectionPoint << " and " << secondIntersectionPoint);

            if (minDist < nearestWallDistance - eps) {
                LOGI_IF(debugSpotSystem, "found a new nearest wall: " << wall << " for distance: " << minDist << " < " << nearestWallDistance);

                nearestWallDistance = minDist;
                nearestWall = wall;
            }
        } else {
            if (wallContainsFirstPoint) LOGI_IF(debugSpotSystem, "\tfound first but not the second");
            if (wallContainsSecondPoint) LOGI_IF(debugSpotSystem, "\tfound second but not the first");
        }
    }
    LOGF_IF(debugSpotSystem && nearestWallDistance == 100000.f, "Couldn't find a wall between points " << firstPoint << " and " << secondPoint);

    LOGI_IF(debugSpotSystem, "\tActive wall is " << nearestWall);
    return nearestWall;
}

void insertInWallsIfNotPresent(std::list<std::pair<glm::vec2, glm::vec2>> & walls, const glm::vec2 & firstPoint,  const glm::vec2 & secondPoint) {
    auto pair = std::make_pair( firstPoint, secondPoint );

    //s'il n'est pas déjà présent, on l'insère dans la liste
    if (std::find(walls.begin(), walls.end(), pair) == walls.end()
        && std::find(walls.begin(), walls.end(), std::make_pair( secondPoint, firstPoint)) == walls.end()) {
        walls.push_back(pair);
    }
}

bool doesVec2ListContainValue(const std::vector<glm::vec2> & list, const glm::vec2 & value) {
    for (auto item : list) {
        if (glm::length2(item - value) < eps) {
            return true;
        }
    }
    return false;
}

void splitIntersectionWalls(std::list<EnhancedPoint> & points) {
    //we don't search two intersection in the same loop, since it could be a mess. When finding an intersection, we restart the algo;
    for (auto it1 = points.begin(); it1 != --points.end(); ++it1) {
        auto it2 = it1;
        for (++it2; it2 != points.end(); ++it2) {

            glm::vec2 intersectionPoint;

            for (auto & endPoint1 : it1->nextEdges) {
                for (auto & endPoint2 : it2->nextEdges) {
                    glm::vec2 startPoint1 = it1->position;
                    glm::vec2 startPoint2 = it2->position;


                    if (IntersectionUtil::lineLine(startPoint1, endPoint1, startPoint2, endPoint2, &intersectionPoint)) {
                        if (glm::length2(intersectionPoint - startPoint2) > eps
                        && glm::length2(intersectionPoint - endPoint1) > eps
                        && glm::length2(intersectionPoint - endPoint1) > eps
                        &&  glm::length2(intersectionPoint - endPoint2) > eps) {

                             LOGI_IF(debugSpotSystem, "Lines " << startPoint1 << " <-> " << endPoint1
                                << " and " << startPoint2 << " <-> " << endPoint2 <<  " are crossing each other at point " << intersectionPoint);


                            auto it = std::find(points.begin(), points.end(), intersectionPoint);
                            //if we couldn't find the intersection point in list, then we create a new point
                            if (it == points.end()) {
                                std::vector<glm::vec2> nexts;
                                nexts.push_back(endPoint1);
                                nexts.push_back(endPoint2);
                                points.push_back(EnhancedPoint(intersectionPoint, nexts, "intersection point"));
                            } else {
                                //todo: vérifier s'il est pas déjà présent avant de l'ajouter
                                // if (std::find(points.begin(), points.end(), endPoint1) == points.end()) {
                                if (! doesVec2ListContainValue(it->nextEdges, endPoint1)) {
                                    it->nextEdges.push_back(endPoint1);
                                }
                                if (! doesVec2ListContainValue(it->nextEdges, endPoint2)) {
                                    it->nextEdges.push_back(endPoint2);
                                }
                            }

                            endPoint1 = intersectionPoint;
                            endPoint2 = intersectionPoint;

                            //restart the algo from start
                            splitIntersectionWalls(points);
                            return;
                        }
                    }
                }
            }
        }
    }
}

void SpotSystem::DoUpdate(float) {
    currentDrawPointIndice = 0;
    currentDrawEdgeIndice = 0;
    currentDrawTriangleColorIndice = 0;
    LOGI_IF(debugSpotSystem, "\n");

    //external walls helper
    float sx = PlacementHelper::ScreenWidth / 2.;
    float sy = PlacementHelper::ScreenHeight / 2.;

    glm::vec2 externalWalls[4] = {
        glm::vec2(-sx, sy), // top left
        glm::vec2(sx, sy), // top right
        glm::vec2(sx, -sy), // bottom right
        glm::vec2(-sx, -sy), // bottom left
    };

    std::list<EnhancedPoint> points;

    //Première étape : on ajoute les points des blocks
    FOR_EACH_ENTITY(Block, block)
        TransformationComponent * tc = TRANSFORM(block);

        glm::vec2 offset = glm::rotate(tc->size / 2.f, tc->rotation);

        glm::vec2 rectanglePoints[4] = {
            tc->position - offset, //first point
            tc->position + offset, //second point
        };
        points.push_back(EnhancedPoint(rectanglePoints[0], rectanglePoints[1],
            theEntityManager.entityName(block) + "- first point"));
        points.push_back(EnhancedPoint(rectanglePoints[1], rectanglePoints[0],
            theEntityManager.entityName(block) + "- second point"));
    }

    // et les points des murs extérieurs
    points.push_back(EnhancedPoint(externalWalls[0], externalWalls[1], "wall top left"));
    points.push_back(EnhancedPoint(externalWalls[1], externalWalls[2], "top right"));
    points.push_back(EnhancedPoint(externalWalls[2], externalWalls[3], "wall bottom right"));
    points.push_back(EnhancedPoint(externalWalls[3], glm::vec2(-sx, -10000.f), "wall bottom left"));

    // si 2 murs se croisent, on crée le point d'intersection et on split les 2 murs en 4 demi-murs
    splitIntersectionWalls(points);

    //on garde la liste de tous les murs disponibles
    std::list<std::pair<glm::vec2, glm::vec2>> walls;
    for (auto point : points) {
        for (auto next : point.nextEdges) {
            insertInWallsIfNotPresent(walls, point.position, next);
        }
    }
    //on ajoute le premier mur à la main parce qu'il est spécial (il va bouger au fil du temps, car il dépend de la caméra)
    insertInWallsIfNotPresent(walls, glm::vec2(-sx, -10000.f), externalWalls[0]);

    auto bottomLeft = std::find(points.begin(), points.end(), "wall bottom left");
    auto wallBotLeft = std::find(walls.begin(), walls.end(), std::make_pair(externalWalls[3], glm::vec2(-sx, -10000.f)));
    auto wallTopLeft = std::find(walls.begin(), walls.end(), std::make_pair(glm::vec2(-sx, -10000.f), externalWalls[0]));
    LOGF_IF(wallBotLeft == walls.end() || wallTopLeft == walls.end(),
        "Can't find left wall?" << (wallBotLeft == walls.end() ? "bottom one" : "") << " && " <<  (wallTopLeft == walls.end() ? "top one" : ""));

    glm::vec2 mousePosition = theTouchInputManager.getTouchLastPosition(0);
    bool isTouched = theTouchInputManager.isTouched(0);

    FOR_EACH_ENTITY_COMPONENT(Spot, e, sc)
        //on vérifie qu'on a déplacé le spot
        sc->dragStarted = isTouched && (sc->dragStarted || IntersectionUtil::pointRectangle(mousePosition, TRANSFORM(e)));

        // if the point has not changed since last time don't recalculate the rays
        if (! sc->dragStarted && sc->highlightedEdges.size() != 0) {
            continue;
        }

        if (sc->dragStarted) {
            // don't go in external walls
            if (sx - glm::abs(mousePosition.x) < TRANSFORM(e)->size.x / 2.f) {
                int sign = mousePosition.x > 0.f ? 1 : -1;
                mousePosition.x = sign * ( sx - TRANSFORM(e)->size.x / 2.f - eps );
            }
            if (sy - glm::abs(mousePosition.y) < TRANSFORM(e)->size.y / 2.f) {
                int sign = mousePosition.y > 0.f ? 1 : -1;
                mousePosition.y = sign * ( sy - TRANSFORM(e)->size.y / 2.f - eps );
            }
            TRANSFORM(e)->position = mousePosition;

            isTouched = false;
        }

        sc->highlightedEdges.clear();

        const glm::vec2 pointOfView = TRANSFORM(e)->position;

        LOGI_IF(debugSpotSystem, "pointOfView: " << pointOfView);
        drawPoint(pointOfView, "pointOfView", Color(1., .8, 0));

        //on change les 3 points qui dépendent de la caméra
        wallTopLeft->first.y = wallBotLeft->second.y = bottomLeft->nextEdges[0].y = pointOfView.y;

        // on trie les points par angle, en sens horaire (min = max = (-1, 0))
        points.sort([pointOfView] (const EnhancedPoint & ep1, const EnhancedPoint & ep2) {
            float firstAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(ep1.position - pointOfView));
            float secondAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(ep2.position - pointOfView));
            return (firstAngle > secondAngle);
        });
        //on trie aussi les murs pour que le premier point rencontré soit le 1er en sens horaire
        for (auto & wall : walls) {
            float firstAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(wall.first - pointOfView));
            float secondAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(wall.second - pointOfView));

            if (secondAngle > firstAngle) {
                auto tmp = wall.first;
                wall.first = wall.second;
                wall.second = tmp;
            }
        }

        //le dernier mur est un peu particulier : pour lui, on inverse les 2 valeurs pour qu'on ait bien une boucle
        auto tmp = wallBotLeft->first;
        wallBotLeft->first = wallBotLeft->second;
        wallBotLeft->second = tmp;

        //on s'assure que le 1er point qui sera parcouru est le "wall middle left"
        points.push_front(EnhancedPoint( glm::vec2(-sx, pointOfView.y), externalWalls[0], "wall middle left (first point)"));

        // on trie les murs par distance à la caméra, du plus proche au plus lointain
        walls.sort([pointOfView] (std::pair<glm::vec2, glm::vec2> & w1, std::pair<glm::vec2, glm::vec2> & w2) {
            float firstDist = distancePointToSegment(w1.first, w1.second, pointOfView);
            float secondDist = distancePointToSegment(w2.first, w2.second, pointOfView);

            return firstDist < secondDist;
        });

        //debug print
        {
            FOR_EACH_ENTITY(Block, block)
                if (IntersectionUtil::pointRectangle(pointOfView, TRANSFORM(block))) {
                    LOGI_IF(debugSpotSystem, "Point of view is INSIDE the block " << theEntityManager.entityName(block));
                }
            }
            int w = 0;
            LOGI_IF(debugSpotSystem, "Walls are: ");
            for (auto wall : walls) {
                LOGI_IF(debugSpotSystem, "\t" <<  ++w << ". " << wall);
            }
            w = 0;
            LOGI_IF(debugSpotSystem, "Points are: ");
            for (auto point : points) {
                LOGI_IF(debugSpotSystem, "\t" << ++w << ". " << point
                    << " (angle=" << glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(point.position - pointOfView)) << " )");
            }
        }

        // le point de départ de l'éclairage
        glm::vec2 startPoint = points.begin()->position;
        auto activeWall = getActiveWall(walls, pointOfView, startPoint, glm::vec2(-sx, pointOfView.y));
        if (! IntersectionUtil::pointLine(startPoint, activeWall.first, activeWall.second)) {
            LOGI_IF(debugSpotSystem, "First point ( " << startPoint << " ) is NOT on the active wall. There must be a block between it and the camera right? So we'll project\
                that point on the active wall and take this point as the start point...");
            if (! IntersectionUtil::lineLine(pointOfView, startPoint, activeWall.first, activeWall.second, & startPoint, true))
                LOGF("could not project!!");
        }

        LOGI_IF(debugSpotSystem, "Start point is " << startPoint);
    #if SAC_DEBUG
        drawPoint(points.begin()->position, points.begin()->name);
    #endif

        //le dernier point de l'éclairage courant
        glm::vec2 endPoint;

        // on commence directement au 2eme du coup vu qu'on a déjà pris le 1er au dessus
        for (auto pointIt = ++points.begin(); pointIt != points.end(); ++pointIt) {
            auto point = *pointIt;

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
                if (isFirstPointOnWall) LOGI_IF(debugSpotSystem, "\t\tfirst cond: true");
                else LOGI_IF(debugSpotSystem, "\t\tfirst cond: false -> " << startPoint << " not on " << activeWall);

                if (isCurrentPointTheEndPoint) LOGI_IF(debugSpotSystem, "\t\tsecond cond: true");
                else LOGI_IF(debugSpotSystem, "\t\tsecond cond: false -> " << activeWall.second << " != " << point.position);

                LOGI_IF(debugSpotSystem, "\tCurrent point is not on the active wall! Projecting point " << point.position << " into the old active wall..." << activeWall);
                IntersectionUtil::lineLine(pointOfView, point.position, activeWall.first, activeWall.second, & endPoint, true);
            }

            //finalement on affiche notre zone à éclairer
            sc->highlightedEdges.push_back(std::make_pair(startPoint, endPoint));


            // maintenant qu'on a fini le mur, il faut chercher le futur mur actif, et projeter notre point dessus
            if (hasEndedCurrentActiveWall) {
                LOGI_IF(debugSpotSystem, "\tWe ended a wall, searching for startPoint of next wall...");
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
                    LOGI_IF(debugSpotSystem,  "\tNext active wall not found, meaning we are at bottom left external wall position");
                    startPoint = point.position;
                } else {
                    // sinon si on arrive pas à projeter sur le mur suivant, il y a un problème
                    if (! IntersectionUtil::lineLine(pointOfView, point.position, nextActiveWall.first, nextActiveWall.second, & startPoint, true)) {
                        LOGF("could not project!!");
                    }
                }

            //il faut regarder si le point courant est visible ou pas. S'il est derrière le mur actif, le point de départ reste sur le mur actif
            } else {
                glm::vec2 intersection;
                bool isIntercepting = IntersectionUtil::lineLine(pointOfView, point.position, activeWall.first, activeWall.second, &intersection);
                if (isIntercepting && glm::length2(intersection - point.position) > eps) {
                    // LOGI_IF(debugSpotSystem, intersection << " != " << point.position);
                    LOGI_IF(debugSpotSystem, "\tCurrent point is behind the activeWall, starting point is the projection point on the active wall");
                    startPoint = intersection;
                //sinon il est la base du nouveau mur actif, donc on met à jour le ponit de départ
                } else {
                    LOGI_IF(debugSpotSystem, "\tStarting point is now this point for next active wall");
                    startPoint = point.position;
                }
            }
            LOGI_IF(debugSpotSystem, "\tNext startPoint is " << startPoint);
        }

        // 2 cas pour le dernier point:
        // 1) soit le dernier point et le 1er point sont un seul mur, et dans ce cas on affiche le mur (mur extérieur par ex)
        // 2) soit ils sont sur 2 murs distincts, et dans ce cas on projete les 2 points sur le mur actif (2 blocks, 1 de chaque côté de l'axe des abcisses)
        activeWall = getActiveWall(walls, pointOfView, startPoint, points.front().position);
        LOGI_IF(debugSpotSystem, "\tLast activeWall is " << activeWall << " so projeting " << startPoint << " and " << points.front().position << " on it.");
        IntersectionUtil::lineLine(pointOfView, startPoint, activeWall.first, activeWall.second, & startPoint, true);
        IntersectionUtil::lineLine(pointOfView, points.front().position, activeWall.first, activeWall.second, & endPoint, true);

        sc->highlightedEdges.push_back(std::make_pair(startPoint, endPoint));

        //finalement, on supprime le premien point de la liste, qui correspond à "middle wall left", et qui dépend du Y de notre spot
        points.pop_front();
    }

    //finalement on affiche tous les triangles
    FOR_EACH_ENTITY_COMPONENT(Spot, e, sc)
        for (auto pair : sc->highlightedEdges) {
            drawTriangle(TRANSFORM(e)->position, pair.first, pair.second);
        }
    }

    //hide the extra
    while (currentDrawPointIndice < (int)drawPointList.size()) {
        RENDERING(drawPointList[currentDrawPointIndice])->show = false;
        ++currentDrawPointIndice;
    }
    while (currentDrawEdgeIndice < (int)drawEdgeList.size()) {
        RENDERING(drawEdgeList[currentDrawEdgeIndice])->show = false;
        ++currentDrawEdgeIndice;
    }
    while (currentDrawTriangleColorIndice < (int)drawTriangleList.size()) {
        RENDERING(drawTriangleList[currentDrawTriangleColorIndice].first)->show = false;
        ++currentDrawTriangleColorIndice;
    }

}


void SpotSystem::DeleteHighlightEntities() {
    while (drawPointList.begin() != drawPointList.end()) {
        theEntityManager.DeleteEntity(*--drawPointList.end());
        drawPointList.pop_back();
    }
    while (drawEdgeList.begin() != drawEdgeList.end()) {
        theEntityManager.DeleteEntity(*--drawEdgeList.end());
        drawEdgeList.pop_back();
    }
    while (drawTriangleList.begin() != drawTriangleList.end()) {
        theEntityManager.DeleteEntity((*--drawTriangleList.end()).first);
        drawTriangleList.pop_back();
    }
}

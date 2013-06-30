#include "SpotSystem.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/BlockSystem.h"
#include "systems/ButtonSystem.h"

#include "util/DrawSomething.h"
#include "util/IntersectionUtil.h"

#include "base/TouchInputManager.h"
#include "base/PlacementHelper.h"

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/projection.hpp>

#include <algorithm>
#include <ostream>
#include <iomanip>

//activate or not logs (debug)
#if SAC_DEBUG
// static bool debugSpotSystem = false;
// static bool debugDistanceCalculation = false;
static bool debugSpotSystem = true;
static bool debugDistanceCalculation = true;
//we use this specific log function for unit tests
#define SPOT_SYSTEM_LOG(lvl, x) {\
    if (lvl & theSpotSystem.FLAGS_ENABLED) {\
        std::ostream out(theSpotSystem.outputStream);\
        out << std::fixed << std::setprecision(1) << x << "\n";\
    }\
}

#else
static bool debugSpotSystem = false;
static bool debugDistanceCalculation = false;

//we use this specific log function for unit tests
#define SPOT_SYSTEM_LOG(lvl, x) {}
#endif

static float FAR_FAR_AWAY = 1000.f;

INSTANCE_IMPL(SpotSystem);



SpotSystem::SpotSystem() : ComponentSystemImpl <SpotComponent>("Spot") {
#if SAC_DEBUG
    //by default, outputStream is stdout
    outputStream = std::cout.rdbuf();
    FLAGS_ENABLED = 0;
#endif
    totalHighlightedDistance2Objective = totalHighlightedDistance2Done = 0.f;
}


inline std::ostream & operator<<(std::ostream & o, const EnhancedPoint & ep) {
    o << "name='" << ep.name << "': position='" << ep.position;
    int i = 0;
    for (auto item : ep.nextEdges)
        o << "' nextEdge" <<  ++i << "='" << item << ", ";
    return o;
}
inline std::ostream & operator<<(std::ostream & o, const Wall & wall) {
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

// retourne le mur actif entre les 2 points vus de la caméra.
Wall getActiveWall(const std::list<Wall> & walls,
    const glm::vec2 & pointOfView, const glm::vec2 & firstPoint, const glm::vec2 & secondPoint) {

    //utile lorsque le point du mur le plus proche de la caméra est l'extremité du VRAI mur actif. C'est pour s'assurer que si on est dans ce
    //cas spécifique, on choisira le vrai mur
    bool nearestWallContainsFirstPointReally = false;
    float nearestWallDistance = FAR_FAR_AWAY;
    Wall nearestWall;

    for (auto wall : walls) {
        // LOGI_IF(debugSpotSystem, "\ttrying wall " << wall.first << " <-> " << wall.second);
        glm::vec2 firstIntersectionPoint, secondIntersectionPoint;

        //le facteur 100 est juste là pour faire une demi droite "infinie"
        bool wallContainsFirstPointReally = IntersectionUtil::lineLine(pointOfView, firstPoint, wall.first, wall.second, &firstIntersectionPoint);
        bool wallContainsFirstPointByProjection = IntersectionUtil::lineLine(pointOfView, firstPoint + 100.f * (firstPoint - pointOfView), wall.first, wall.second, &firstIntersectionPoint);
        bool wallContainsSecondPoint = IntersectionUtil::lineLine(pointOfView, secondPoint + 100.f * (secondPoint - pointOfView), wall.first, wall.second, &secondIntersectionPoint);

        if (wallContainsFirstPointByProjection && wallContainsSecondPoint) {
            float minDist = glm::min(glm::length2(firstIntersectionPoint - pointOfView), glm::length2(secondIntersectionPoint - pointOfView));
            LOGI_IF(debugSpotSystem, "\t\t Found a candidate wall: " << wall << " for distance: " << minDist
                << " points: " << firstIntersectionPoint << " and " << secondIntersectionPoint);

            bool foundABetterWall = false;

            //si notre mur contient le premier point et pas l'ancien mur alors il est nécessairement mieux
            if (wallContainsFirstPointReally && !nearestWallContainsFirstPointReally) {
                LOGI_IF(debugSpotSystem, "\t\t  Since the current active wall didn't contain the first point, " << wall <<  " is preferred");
                foundABetterWall = true;
            // si la distance est identique pour les 2 murs, alors le point le plus proche est une extremité
            // et il faut qu'on regarde lequel est devant
            } else if (glm::abs(minDist - nearestWallDistance) <  eps) {
                 glm::vec2 nearestPoint = (glm::length2(firstIntersectionPoint - pointOfView) <
                    glm::length2(secondIntersectionPoint - pointOfView)) ?
                        firstIntersectionPoint : secondIntersectionPoint;

                auto notThePivot = (glm::length2(nearestWall.first - nearestPoint) < eps) ? nearestWall.second : nearestWall.first;

                LOGF_IF(glm::length2(nearestPoint - notThePivot) < eps, nearestPoint << " " << notThePivot);

                foundABetterWall = IntersectionUtil::lineLine(pointOfView, notThePivot, wall.first, wall.second, 0);

                LOGI_IF(debugSpotSystem, "\t\t  A wall is at the same distance, " << (foundABetterWall ? "and is above" : "but is behind") << " the nearestwall");
            // le cas de base: s'il est plus proche, il est mieux
            } else if (minDist < nearestWallDistance - eps) {
                LOGI_IF(debugSpotSystem, "\t\t  Found a new nearest wall: " << wall << " for distance: " << minDist << " < " << nearestWallDistance <<
                    "(" << minDist << "<" << nearestWallDistance - eps <<")");
                foundABetterWall = true;
            }


            if (foundABetterWall) {
                nearestWallDistance = minDist;
                nearestWall = wall;
                nearestWallContainsFirstPointReally = wallContainsFirstPointReally;
            }
        } else {
            if (wallContainsFirstPointByProjection) LOGE_IF(debugSpotSystem, "\t\t Wall contains first but not the second");
            if (wallContainsSecondPoint) LOGE_IF(debugSpotSystem, "\t\t Wall contains second but not the first");
        }
    }
    LOGF_IF(debugSpotSystem && nearestWallDistance == FAR_FAR_AWAY, "Couldn't find a wall between points " << firstPoint << " and " << secondPoint);

    LOGI_IF(debugSpotSystem, "\tActive wall is " << nearestWall);
    return nearestWall;
}

bool doesVec2ListContainValue(const std::vector<glm::vec2> & list, const glm::vec2 & value) {
    for (auto item : list) {
        if (glm::length2(item - value) < eps) {
            return true;
        }
    }
    return false;
}

bool insertInPointsIfNotPresent(std::list<EnhancedPoint> & points, const EnhancedPoint & ep) {
    auto it = std::find(points.begin(), points.end(), ep.position);

    // il n'y est pas, on peut le créer
    if (it == points.end()) {
        points.push_back(ep);
        return true;
    // sinon, on merge les sommets 'nextEdges' avec ceux déjà existants
    } else {
        for (auto next : ep.nextEdges) {
            if (! doesVec2ListContainValue(it->nextEdges, next)) {
                it->nextEdges.push_back(next);
            }
        }
        return false;
    }
}

bool insertInWallsIfNotPresent(std::list<Wall> & walls, const glm::vec2 & firstPoint,  const glm::vec2 & secondPoint) {
    auto pair = Wall( firstPoint, secondPoint );
    // LOGI("try to insert: " << pair);

    //s'il n'est pas déjà présent, on l'insère dans la liste
    if (std::find(walls.begin(), walls.end(), pair) == walls.end() &&
        std::find(walls.begin(), walls.end(), Wall( secondPoint, firstPoint)) == walls.end()) {

        walls.push_back(pair);
        // LOGI("successfully");
        return true;
    }
    return false;
}

bool splitIntersectionWalls(std::list<EnhancedPoint> & points) {
    //we don't search two intersection in the same loop, since it could be a mess. When finding an intersection, we should restart the algo;
    for (auto it1 = points.begin(); it1 != --points.end(); ++it1) {
        auto it2 = it1;
        for (++it2; it2 != points.end(); ++it2) {

            glm::vec2 intersectionPoint;

            for (auto & endPoint1 : it1->nextEdges) {
                for (auto & endPoint2 : it2->nextEdges) {
                    glm::vec2 startPoint1 = it1->position;
                    glm::vec2 startPoint2 = it2->position;


                    if (IntersectionUtil::lineLine(startPoint1, endPoint1, startPoint2, endPoint2, &intersectionPoint)) {
                        if (glm::length2(intersectionPoint - startPoint1) > eps
                        && glm::length2(intersectionPoint - startPoint2) > eps
                        && glm::length2(intersectionPoint - endPoint1) > eps
                        &&  glm::length2(intersectionPoint - endPoint2) > eps) {

                             LOGI_IF(debugSpotSystem, "Lines " << startPoint1 << " <-> " << endPoint1
                                << " and " << startPoint2 << " <-> " << endPoint2 <<  " are crossing each other at point " << intersectionPoint);


                            auto it = std::find(points.begin(), points.end(), intersectionPoint);
                            //if we couldn't find the intersection point in list, then we create a new point
                            if (it == points.end()) {
                                LOGI_IF(debugSpotSystem, "Point " << intersectionPoint << " does not exist yet; creating it");

                                std::vector<glm::vec2> nexts;
                                nexts.push_back(endPoint1);
                                nexts.push_back(endPoint2);
                                insertInPointsIfNotPresent(points, EnhancedPoint(intersectionPoint, nexts, "intersection point", true));
                            } else {

                                // for (auto item : points) {
                                //     LOGI(item.name << " and " << item.position);
                                // }


                                LOGI_IF(debugSpotSystem, "Point " << *it << " is already in list; adding next edges to it");
                                if (! doesVec2ListContainValue(it->nextEdges, endPoint1)) {
                                    LOGI_IF(debugSpotSystem, "Point " << endPoint1 << " not in its list");
                                    it->nextEdges.push_back(endPoint1);
                                } else {
                                    LOGI_IF(debugSpotSystem, "Point " << endPoint1 << " already in its list");
                                }

                                if (! doesVec2ListContainValue(it->nextEdges, endPoint2)) {
                                    LOGI_IF(debugSpotSystem, "Point " << endPoint2 << " not in its list");
                                    it->nextEdges.push_back(endPoint2);
                                } else {
                                    LOGI_IF(debugSpotSystem, "Point " << endPoint2 << " already in its list");
                                }
                            }

                            //THIS CODE IS DOING SOMETHING! Since endPoint1 and endPoint2 are references, we are modyfing their values by doing this. These variables
                            // ARE NOT just local :-)
                            endPoint1 = endPoint2 = intersectionPoint;
                            //restart the algo from start
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void getAllWallsExtremities( std::list<EnhancedPoint> & points, const glm::vec2 externalWalls[4]) {
    //Première étape : on ajoute les points des blocks
    FOR_EACH_ENTITY_COMPONENT(Block, block, bc)
        TransformationComponent * tc = TRANSFORM(block);

        //since we consider only 2 points, don't consider size.y
        glm::vec2 offset = glm::rotate(.5f * glm::vec2(tc->size.x, 0.), tc->rotation);

        glm::vec2 rectanglePoints[4] = {
            tc->position - offset, //first point
            tc->position + offset, //second point
        };
        insertInPointsIfNotPresent(points, EnhancedPoint(rectanglePoints[0], rectanglePoints[1],
            theEntityManager.entityName(block) + "- first point", bc->isDoubleFace));
        insertInPointsIfNotPresent(points, EnhancedPoint(rectanglePoints[1], rectanglePoints[0],
            theEntityManager.entityName(block) + "- second point", bc->isDoubleFace));
    END_FOR_EACH()

    // et les points des murs extérieurs - pas besoin de vérifier pour eux
    points.push_back(EnhancedPoint(externalWalls[0], externalWalls[1], "wall top left", false));
    points.push_back(EnhancedPoint(externalWalls[1], externalWalls[2], "top right", false));
    points.push_back(EnhancedPoint(externalWalls[2], externalWalls[3], "wall bottom right", false));
    points.push_back(EnhancedPoint(externalWalls[3], glm::vec2(-PlacementHelper::ScreenSize.x / 2., FAR_FAR_AWAY), "wall bottom left", false));
}

const Wall getUnion(const Wall & wall, const Wall & zone) {
    //order by x
    auto merge = wall;

    const float firstPointDot = glm::dot (zone.first - wall.first, wall.second - wall.first) / glm::length2(wall.second - wall.first);
    const float secondPointDot = glm::dot (zone.second - wall.first, wall.second - wall.first) / glm::length2(wall.second - wall.first);
    //if the point is NOT on the segment yet, we can increase it ...
    if (firstPointDot < 0.f) {
        merge.first = zone.first;
    } else if (firstPointDot > 1.f) {
        merge.second = zone.first;
    }

    if (secondPointDot < 0.f) {
        merge.first = zone.second;
    } else if (secondPointDot > 1.f) {
        merge.second = zone.second;
    }

    LOGI_IF(debugSpotSystem || debugDistanceCalculation, "  final union is " << merge << " for " << wall << " and " << zone << "( " << firstPointDot << ")");

    return merge;
}

float calculateHighlightedZone(std::list<Wall> & highlightedEdges) {
    float result = 0.f;

    // cette liste contient les morceaux de murs éclairés, on la remplit au fur et à mesure
    highlightedEdges.clear();


    //on parcoure chaque spot
    FOR_EACH_ENTITY_COMPONENT(Spot, e, sc)
        auto position = TRANSFORM(e)->position;
        LOGI_IF(debugSpotSystem || debugDistanceCalculation, "\n\nConsidering spot at position " << position);

        //et l'ensemble des "zones" (murs à vrai dire) qu'il éclaire
        for (auto zone : sc->highlightedEdges) {
            LOGI_IF(debugSpotSystem || debugDistanceCalculation, "\nConsidering zone " << zone);

            //cette liste contient tous les murs DÉJÀ ÉCLAIRÉS, qui sont dans la continuité
            //du mur actuel
            std::list<Wall> wallsMatching;


            //si la liste ci dessus est non vide, il va falloir merger tous ces segments en un unique
            bool needAMerge = false;

            //pour la remplir, on parcourt les segments déjà éclairés
            for (auto wall = highlightedEdges.begin(); wall != highlightedEdges.end(); ++wall) {
                glm::vec2 intersectionPoint;

                //et si ils sont paralléles ET coincidents, on l'ajoute à la liste
                if (IntersectionUtil::lineLine(wall->first, wall->second, zone.first, zone.second, &intersectionPoint)) {
                    // LOGI_IF(debugSpotSystem || debugDistanceCalculation, "wall new candidate: " << *wall);
                    //il y a 2 cas où la fonction renvoie vrai:
                    //1) s'ils sont perpendiculaires avec un point pivot en commun (à ignorer)
                    //2) s'ils sont adjacents / ont une partie confondu
                    auto absDot = glm::abs(glm::dot(wall->second - wall->first, zone.second - zone.first));
                    // LOGI(glm::length(wall->second - wall->first)
                        // << " " << glm::length(zone.second - zone.first) << " " << absDot);


                    // if (absDot < eps) {
                    if (glm::length(wall->second - wall->first) * glm::length(zone.second - zone.first) - absDot > eps) {
                        LOGW_IF(debugSpotSystem || debugDistanceCalculation, " -> Intersect but not parallels with " << *wall);
                        continue;
                    }
                    needAMerge = true;
                    LOGI_IF(debugSpotSystem || debugDistanceCalculation, " -> Yes! " << *wall);
                    wallsMatching.push_back(*wall);

                    //on supprime le segment de la liste, car on va y mettre à la place la fusion avec le nouvel élément
                    highlightedEdges.erase(wall++);
                } else {
                    LOGE_IF(debugSpotSystem || debugDistanceCalculation, " -> NO intersection with " << *wall);
                }
            }

            //donc si on a trouvé un morceau du même mur déjà éclairé, on les merge
            if (needAMerge) {
                float beforeUnionTotalDistance = 0.f;


                Wall unionned = zone;
                for (auto wall = wallsMatching.begin(); wall != wallsMatching.end(); ++wall) {
                    //utilisé plus bas pour connaître le gain
#if SAC_DEBUG
                    beforeUnionTotalDistance += glm::length(wall->first - wall->second);
#else
                    beforeUnionTotalDistance += glm::length2(wall->first - wall->second);
#endif

                    LOGI_IF(debugSpotSystem || debugDistanceCalculation, "\tfusionning " << unionned << " and " << *wall);
                    unionned = getUnion(*wall, unionned);
                }

                //on calcule le gain de distance, qui correspond à la  différence entre
                //la distance de l'union des segments moins la somme des distances
                //des anciennes segments séparés
#if SAC_DEBUG
                float afterUnionTotalDistance = glm::length(unionned.first - unionned.second);
#else
                float afterUnionTotalDistance = glm::length2(unionned.first - unionned.second);
#endif
                //finalement on rajoute le segment à la liste des murs éclairés
                highlightedEdges.push_back(unionned);

                //du debug uniquement
                if (glm::abs(beforeUnionTotalDistance - afterUnionTotalDistance) < eps)  {
                    LOGI_IF(debugSpotSystem || debugDistanceCalculation, "***Skipped*** (already highlighted)");
                } else {
                    LOGI_IF(debugSpotSystem || debugDistanceCalculation, "***Merged*** (" << unionned << ")for a bonus of " << afterUnionTotalDistance - beforeUnionTotalDistance);
                    result += afterUnionTotalDistance - beforeUnionTotalDistance;
                }
            //sinon il était tout seul sur ce mur, on l'ajoute normalement
            } else {
                LOGI_IF(debugSpotSystem || debugDistanceCalculation, "***Added***" );
#if SAC_DEBUG
                result += glm::length(zone.first - zone.second);
#else
                result += glm::length2(zone.first - zone.second);
#endif

                highlightedEdges.push_back(Wall( zone.first, zone.second ));
            }
            // SPOT_SYSTEM_LOG(SpotSystem::CALCULATION_ALGO, "current total: " << result);
            // LOGI_IF(debugSpotSystem || debugDistanceCalculation, "\t\tcurrent total: " << result);
        }
    END_FOR_EACH()
    return result;
}

float SpotSystem::recalculateTotalDistanceObjective() {
    totalHighlightedDistance2Objective = 0.f;
    PrepareAlgorithm();
    return totalHighlightedDistance2Objective;
}

void SpotSystem::PrepareAlgorithm() {
    walls.clear();
    points.clear();

    //external walls helper
    float sx = PlacementHelper::ScreenSize.x / 2.;
    float sy = PlacementHelper::ScreenSize.y / 2.;

    glm::vec2 externalWalls[4] = {
        glm::vec2(-sx, sy), // top left
        glm::vec2(sx, sy), // top right
        glm::vec2(sx, -sy), // bottom right
        glm::vec2(-sx, -sy), // bottom left
    };

    // distance total de mur éclairé réalisée
    totalHighlightedDistance2Done = 0.f;

    getAllWallsExtremities(points, externalWalls);

    // si 2 murs se croisent, on crée le point d'intersection et on split les 2 murs en 4 demi-murs
    for (auto item : points) {
        SPOT_SYSTEM_LOG(INTERSECTIONS_SPLIT, item);
    }
    SPOT_SYSTEM_LOG(INTERSECTIONS_SPLIT, "before splitIntersectionWalls");
    while (splitIntersectionWalls(points))
    ;
    SPOT_SYSTEM_LOG(INTERSECTIONS_SPLIT, "after splitIntersectionWalls");
    for (auto item : points) {
        SPOT_SYSTEM_LOG(INTERSECTIONS_SPLIT, item);
    }

    bool shouldUpdateDistanceObjective = false;
    // la distance totale de murs à éclairer
    if (totalHighlightedDistance2Objective < eps) {
        totalHighlightedDistance2Objective = 0.f;
        shouldUpdateDistanceObjective = true;
    }

    for (auto point : points) {
        for (auto next : point.nextEdges) {
            if (insertInWallsIfNotPresent(walls, point.position, next)) {
                if (shouldUpdateDistanceObjective) {
                    //double distance if the wall is visible from the 2 sides
                    int doubled = (point.isDoubleFace && std::find(points.begin(), points.end(), next)->isDoubleFace) ? 2 : 1;
#if SAC_DEBUG
                    totalHighlightedDistance2Objective += doubled * glm::length(next - point.position);
#else
                    totalHighlightedDistance2Objective += doubled * glm::length2(next - point.position);
#endif
                }
            }
        }
    }

    if (shouldUpdateDistanceObjective) {
        auto bottomLeft = std::find(points.begin(), points.end(), "wall bottom left");

        // comme le mur 'wall bottom left' est spécial en Y, on recalcule à la main cette dernière distance
#if SAC_DEBUG
        totalHighlightedDistance2Objective += (2 * sy) - glm::length(bottomLeft->position - bottomLeft->nextEdges[0]);
#else
        totalHighlightedDistance2Objective += (2 * sy) * (2 * sy) - glm::length2(bottomLeft->position - bottomLeft->nextEdges[0]);
#endif
    }
    LOGF_IF(totalHighlightedDistance2Objective < 0, "wut");
}

void SpotSystem::DoUpdate(float) {
    Draw::DrawPointRestart("SpotSystem");
    Draw::DrawVec2Restart("SpotSystem");
    Draw::DrawTriangleRestart("SpotSystem");

    LOGI_IF(debugSpotSystem || debugDistanceCalculation, "\n\n\n");

    //external walls helper
    float sx = PlacementHelper::ScreenSize.x / 2.;
    float sy = PlacementHelper::ScreenSize.y / 2.;

    glm::vec2 externalWalls[4] = {
        glm::vec2(-sx, sy), // top left
        glm::vec2(sx, sy), // top right
        glm::vec2(sx, -sy), // bottom right
        glm::vec2(-sx, -sy), // bottom left
    };

    PrepareAlgorithm();

    //on ajoute le premier mur à la main parce qu'il est spécial (il va bouger au fil du temps, car il dépend de la caméra)
    insertInWallsIfNotPresent(walls, glm::vec2(-sx, FAR_FAR_AWAY), externalWalls[0]);

    auto bottomLeft = std::find(points.begin(), points.end(), "wall bottom left");
    auto wallBotLeft = std::find(walls.begin(), walls.end(), Wall(externalWalls[3], glm::vec2(-sx, FAR_FAR_AWAY)));
    auto wallTopLeft = std::find(walls.begin(), walls.end(), Wall(glm::vec2(-sx, FAR_FAR_AWAY), externalWalls[0]));
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

        SPOT_SYSTEM_LOG(POINTS_ORDER, "Spot " << theEntityManager.entityName(e));

        LOGI_IF(debugSpotSystem, "pointOfView: " << pointOfView);
        Draw::DrawPoint("SpotSystem", pointOfView, Color(1., .8, 0), "pointOfView");

        //on change les 3 points qui dépendent de la caméra
        wallTopLeft->first.y = wallBotLeft->second.y = bottomLeft->nextEdges[0].y = pointOfView.y;

        // on trie les points par angle, en sens horaire (min = max = (-1, 0))
        points.sort([pointOfView] (const EnhancedPoint & ep1, const EnhancedPoint & ep2) {
            float firstAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(ep1.position - pointOfView));
            float secondAngle = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(ep2.position - pointOfView));

            if (glm::abs(firstAngle - secondAngle) > eps) {
                LOGI_IF(debugSpotSystem, "Angle sorting: " << ep1.position << " and " << ep2.position << ": " << std::fixed << std::setprecision(5) << firstAngle << " " << secondAngle);
                return (firstAngle > secondAngle);
            }
            LOGI_IF(debugSpotSystem, "Angle sorting: that was close! " << ep1.position << " and " << ep2.position);
            return (glm::length2(ep1.position - pointOfView) < glm::length2(ep2.position - pointOfView));
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
        points.push_front(EnhancedPoint( glm::vec2(-sx, pointOfView.y), externalWalls[0], "wall middle left (first point)", false));

        // on trie les murs par distance à la caméra, du plus proche au plus lointain
        walls.sort([pointOfView] (Wall & w1, Wall & w2) {
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
            END_FOR_EACH()
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
        Draw::DrawPoint("SpotSystem", points.begin()->position, Color(1., 1., 1.), points.begin()->name);
    #endif

        //le dernier point de l'éclairage courant
        glm::vec2 endPoint;

        SPOT_SYSTEM_LOG(POINTS_ORDER, points.begin()->name);
        SPOT_SYSTEM_LOG(ACTIVE_WALL, "Active wall is " << activeWall);
        // on commence directement au 2eme du coup vu qu'on a déjà pris le 1er au dessus
        for (auto pointIt = ++points.begin(); pointIt != points.end(); ++pointIt) {
            auto point = *pointIt;

            SPOT_SYSTEM_LOG(POINTS_ORDER, point.name);

            LOGI_IF(debugSpotSystem, "Current point is " << point.name << " ( " << point.position <<  " ) ");

#if SAC_DEBUG
            Draw::DrawPoint("SpotSystem", point.position, Color(1., 1., 1.), point.name);
#endif

            // on cherche le mur actif (c'est à dire le mur le plus proche qui contient le startPoint ET le point actuel)
            activeWall = getActiveWall(walls, pointOfView, startPoint, point.position);
            SPOT_SYSTEM_LOG(ACTIVE_WALL, "Active wall is " << activeWall);

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
                if (isFirstPointOnWall) LOGI_IF(debugSpotSystem, "\t\tisFirstPointOnWall: true");
                else LOGW_IF(debugSpotSystem, "\t\tisFirstPointOnWall: false -> " << startPoint << " not on " << activeWall);

                if (isCurrentPointTheEndPoint) LOGI_IF(debugSpotSystem, "\t\tisCurrentPointTheEndPoint: true");
                else LOGW_IF(debugSpotSystem, "\t\tisCurrentPointTheEndPoint: false -> " << activeWall.second << " != " << point.position);

                LOGI_IF(debugSpotSystem, "\tCurrent point is not on the active wall! Projecting point " << point.position << " into the active wall..." << activeWall);
                IntersectionUtil::lineLine(pointOfView, point.position, activeWall.first, activeWall.second, & endPoint, true);
            }

            //finalement on affiche notre zone à éclairer
            LOGI_IF(debugSpotSystem, "registering " << Wall(startPoint, endPoint));
            sc->highlightedEdges.push_back(Wall(startPoint, endPoint));


            // maintenant qu'on a fini le mur, il faut chercher le futur mur actif, et projeter notre point dessus
            if (hasEndedCurrentActiveWall) {
                LOGI_IF(debugSpotSystem, "\tWe ended a wall, searching for startPoint of next wall...");
                auto nextPointIt = pointIt;
                ++nextPointIt;

                Wall nextActiveWall;
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
                //sinon il est la base du nouveau mur actif, donc on met à jour le point de départ
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
        SPOT_SYSTEM_LOG(ACTIVE_WALL, "Active wall is " << activeWall);
        LOGI_IF(debugSpotSystem, "\tLast activeWall is " << activeWall << " so projeting " << startPoint << " and " << points.front().position << " on it.");
        IntersectionUtil::lineLine(pointOfView, startPoint, activeWall.first, activeWall.second, & startPoint, true);
        IntersectionUtil::lineLine(pointOfView, points.front().position, activeWall.first, activeWall.second, & endPoint, true);
        LOGI_IF(debugSpotSystem, "registering " << Wall(startPoint, endPoint));
        sc->highlightedEdges.push_back(Wall(startPoint, endPoint));

        //finalement, on supprime le premier point de la liste, qui correspond à "middle wall left", et qui dépend du Y de notre spot
        points.pop_front();
    END_FOR_EACH()

    //only debug
    FOR_EACH_ENTITY_COMPONENT(Spot, e, sc)
        float totalDist = 0.f;
        SPOT_SYSTEM_LOG(HIGHLIGHTED_WALLS_BEFORE_MERGE, "Spot at position " << TRANSFORM(e)->position);
        for (auto pair : sc->highlightedEdges) {
            SPOT_SYSTEM_LOG(HIGHLIGHTED_WALLS_BEFORE_MERGE, pair)
            LOGI_IF(debugSpotSystem || debugDistanceCalculation, "highlighted: " << pair);
            totalDist += glm::length2(pair.first - pair.second);
        }
        SPOT_SYSTEM_LOG(HIGHLIGHTED_WALLS_BEFORE_MERGE, "Total distance: " << totalDist)
    END_FOR_EACH()

    totalHighlightedDistance2Done = calculateHighlightedZone(highlightedEdgesFromAllSpots);
    SPOT_SYSTEM_LOG(CALCULATION_ALGO, "totalHighlightedDistance2Objective=" << totalHighlightedDistance2Objective
     << " and totalHighlightedDistance2Done=" << totalHighlightedDistance2Done);

    for (auto pair : highlightedEdgesFromAllSpots) {
        SPOT_SYSTEM_LOG(CALCULATION_ALGO, "highlighted: " << pair);
        LOGI_IF(debugSpotSystem || debugDistanceCalculation, "highlighted: " << pair);
    }

    //finalement on affiche tous les triangles par spot
    FOR_EACH_ENTITY_COMPONENT(Spot, e, sc)
        for (auto pair : sc->highlightedEdges) {
            Draw::DrawTriangle("SpotSystem", TRANSFORM(e)->position, pair.first, pair.second, sc->highlightColor);
        }
    END_FOR_EACH()
}


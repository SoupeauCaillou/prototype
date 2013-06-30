#pragma once

#include "systems/System.h"

#include <glm/gtx/norm.hpp>

#include <ostream>

//used in the algorithm
const float eps = 0.0001f;

struct Wall {
    Wall() : first(0.), second(0.) {}
    Wall(const glm::vec2 & inF, const glm::vec2 & inS) : first(inF), second(inS) {}
    Wall(const Wall & inW) : first(inW.first), second(inW.second) {}

    Wall & operator=(const Wall & inWall) {
        //if (inWall != *this) {
            this->first = inWall.first;
            this->second = inWall.second;
        //}

        return *this;
     }

    bool operator== (const Wall & inWall) const {
        return (glm::length2(first - inWall.first ) + glm::length2(second - inWall.second)) < eps;
    }

    //basically Wall struct is a std::pair...
    glm::vec2 first;
    glm::vec2 second;
};
/*inline bool operator!=(, const Wall & inWall2) {
}*/

struct EnhancedPoint {
    EnhancedPoint() :
        position(0.), name("unknown"), isDoubleFace(false) {}
    EnhancedPoint(const glm::vec2& inp, const glm::vec2 & ne, const std::string & iname, bool isD) :
        position(inp), name(iname), isDoubleFace(isD) { nextEdges.push_back(ne); }
    EnhancedPoint(const glm::vec2& inp, const std::vector<glm::vec2> & ine, const std::string & iname, bool isD) :
        position(inp), nextEdges(ine), name(iname), isDoubleFace(isD) {}


    //old position for the spot (used in case of drag)
    glm::vec2 position;

    //list of all connected points
    std::vector<glm::vec2> nextEdges;

    //only for debug
    std::string name;

    //for solution calculation
    bool isDoubleFace;

    bool operator== (const EnhancedPoint & ep) const {
        return (glm::length2(position - ep.position) < eps);
    }
    bool operator== (const glm::vec2 & pos) const {
        return (glm::length2(position - pos) < eps);
    }
    bool operator== (const std::string & inName) const {
        return (name == inName);
    }
};

struct SpotComponent {
    SpotComponent() : dragStarted(false), highlightColor(Color::random()) {
        highlightColor.b = 0.;
        highlightColor.a = .3;
    }

    bool dragStarted;

    Color highlightColor;

    //for each wall highlighted, get the first and last highlighted points
    std::list<Wall> highlightedEdges;
};

#define theSpotSystem SpotSystem::GetInstance()
#define SPOT(e) theSpotSystem.Get(e)

UPDATABLE_SYSTEM(Spot)
    public:
        float totalHighlightedDistance2Objective, totalHighlightedDistance2Done;

        float recalculateTotalDistanceObjective();

        // cette liste contient les morceaux de murs éclairés, on la remplit au fur et à mesure
        std::list<Wall> highlightedEdgesFromAllSpots;

#if SAC_DEBUG
        //should NOT be here on the release version obv
        std::streambuf * outputStream;
        const static int POINTS_ORDER = 1 << 0; // affiche l'ordre dans lequel les points sont parcourus
        const static int HIGHLIGHTED_WALLS_BEFORE_MERGE = 1 << 1; //todo
        const static int CALCULATION_ALGO = 1 << 2; //todo
        const static int ACTIVE_WALL = 1 << 3; //affiche le mur actif entre chaque point
        const static int INTERSECTIONS_SPLIT = 1 << 4; //affiche les différents murs avant et après l'algo de split

        int FLAGS_ENABLED;
#endif
    private:
        void PrepareAlgorithm();

        // la liste de tous les points intéréssants pour l'algo (tous les sommets)
        std::list<EnhancedPoint> points;
        // la liste de tous les murs disponibles
        std::list<Wall> walls;

};

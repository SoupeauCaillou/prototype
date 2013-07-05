/*
    This file is part of Prototype.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Prototype is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Prototype is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Prototype.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "systems/System.h"

#include <glm/gtx/norm.hpp>

#include <ostream>

//used in the algorithm
const float eps = 0.0001f;

struct Wall {
    Wall() :
        first(0.), second(0.), isDoubleFace(false), isVisibleFromTopOrLeft(false), name("") {}
    Wall(const glm::vec2 & inF, const glm::vec2 & inS, bool idf, bool ivftor, const std::string & isn) :
        first(inF), second(inS), isDoubleFace(idf), isVisibleFromTopOrLeft(ivftor), name(isn) {}
    Wall(const Wall & inW, bool idf, bool ivftor, const std::string & isn) :
        first(inW.first), second(inW.second), isDoubleFace(idf), isVisibleFromTopOrLeft(ivftor), name(isn) {}

    Wall & operator=(const Wall & inWall) {
        if (&inWall != this) {
            this->first = inWall.first;
            this->second = inWall.second;
            this->isDoubleFace = inWall.isDoubleFace;
            this->isVisibleFromTopOrLeft = inWall.isVisibleFromTopOrLeft;
        }

        return *this;
     }
    bool operator== (const std::string& name) const {
        return this->name == name;
    }

    bool operator== (const Wall & inWall) const {

        return (inWall.isDoubleFace == isDoubleFace
        && (isDoubleFace || inWall.isVisibleFromTopOrLeft == isVisibleFromTopOrLeft)
        && glm::length2(first - inWall.first ) + glm::length2(second - inWall.second) < eps);
    }

    //basically Wall struct is a std::pair...
    glm::vec2 first;
    glm::vec2 second;

    //but with visibility
    bool isDoubleFace, isVisibleFromTopOrLeft;

    std::string name;
};

struct EnhancedPoint {
    EnhancedPoint() :
        position(0.), name("unknown"), isDoubleFace(false) {}
    EnhancedPoint(const glm::vec2& inp, const std::string & iname, bool isD) :
        position(inp), name(iname), isDoubleFace(isD) { }
    EnhancedPoint(const glm::vec2& inp, const EnhancedPoint & ne, const std::string & iname, bool isD) :
        position(inp), name(iname), isDoubleFace(isD) { nextEdges.push_back(ne); }
    EnhancedPoint(const glm::vec2& inp, const std::vector<EnhancedPoint> & ine, const std::string & iname, bool isD) :
        position(inp), nextEdges(ine), name(iname), isDoubleFace(isD) {}


    //old position for the spot (used in case of drag)
    glm::vec2 position;

    //list of all connected points
    std::vector<EnhancedPoint> nextEdges;

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
        void PrepareAlgorithm();

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
        bool useOptimization;
    private:
        void getAllWallsExtremities( );
        bool splitIntersectionWalls();
        bool insertInWallsIfNotPresent(const glm::vec2 & firstPoint,  const glm::vec2 & secondPoint,
            bool isDoubleFace, const std::string & name);
        void updateWallsVisibility(const glm::vec2 & pointOfView);
        const Wall & getActiveWall(const glm::vec2 & pointOfView, const glm::vec2 & firstPoint, const glm::vec2 & secondPoint) const;
        bool insertInPointsIfNotPresentOtherwiseMerge(const EnhancedPoint & ep);
        float calculateHighlightedZone();

        // la liste de tous les points intéréssants pour l'algo (tous les sommets)
        std::list<EnhancedPoint> points;
        // la liste de tous les murs disponibles
        std::list<Wall> walls;
        std::vector<EnhancedPoint> externalWalls;
};

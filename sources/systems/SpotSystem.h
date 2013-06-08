#pragma once

#include "systems/System.h"

#include <glm/gtx/norm.hpp>

//used in the algorithm
const float eps = 0.0001f;
struct EnhancedPoint {
    EnhancedPoint() :
        position(0.), name("unknown") {}
    EnhancedPoint(const glm::vec2& inp, const glm::vec2 & ne, const std::string & iname) :
        position(inp), name(iname) { nextEdges.push_back(ne); }
    EnhancedPoint(const glm::vec2& inp, const std::vector<glm::vec2> & ine, const std::string & iname) :
        position(inp), nextEdges(ine), name(iname) {}


    //old position for the spot (used in case of drag)
    glm::vec2 position;

    //list of all connected points
    std::vector<glm::vec2> nextEdges;

    //only fpr debug
    std::string name;

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
        highlightColor.a = .3;
    }

    bool dragStarted;

    Color highlightColor;

    //for each wall highlighted, get the first and last highlighted points
    std::list<std::pair<glm::vec2, glm::vec2>> highlightedEdges;
};

#define theSpotSystem SpotSystem::GetInstance()
#define SPOT(e) theSpotSystem.Get(e)

UPDATABLE_SYSTEM(Spot)
    public:
        float totalHighlightedDistance2Objective, totalHighlightedDistance2Done;

};

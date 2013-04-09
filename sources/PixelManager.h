#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>

#include "base/Game.h"
#include "base/GameContext.h"

#include <glm/glm.hpp>

#include "systems/RenderingSystem.h"

#include "util/ImageLoader.h"

struct pixel {
    static pixel Default;
    pixel(Entity p=0, bool enabled=false):p(p), enabled(enabled) {}
    Entity p;
    glm::vec2 finalPosition;
    Color finalColor;
    glm::vec2 finalSize;
    bool enabled;
    
    bool operator==(pixel a){
    if (p == a.p)
        return true;
    return false;
    }
};

inline std::ostream& operator<<(std::ostream& s, const pixel& v){
    s << "p = " << v.p << std::endl;
    s << "finalPosition = " << v.finalPosition.x << ":"<< v.finalPosition.y << std::endl;
    s << "finalColor = " << v.finalColor << std::endl;
    s << "finalSize = " << v.finalSize.x << ":" << v.finalSize.y << std::endl;
    s << "enabled = " << v.enabled << std::endl;
    return s;
}

class PixelManager {
    public:
        static const int DIVIDE_BY;
        static const float SPEED;

        PixelManager(std::string assetName, AssetAPI *assetapi);
        virtual ~PixelManager();

        pixel& findPixel(glm::vec2 pos);

        void updatePixel();

        bool changeBackGround(std::string assetName);

        void clickedOn(glm::vec2 position);
    private:
        bool splitPixel(pixel& p);

        bool fusePixel(pixel& p);

        Color averageColor(glm::vec2 size, glm::vec2 position);

        AssetAPI *assetAPI;
        ImageDesc bg;
        
        std::list<pixel> pixels;
};

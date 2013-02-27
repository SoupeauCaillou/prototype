#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>

#include "base/MathUtil.h"
#include "base/Game.h"

#include "systems/RenderingSystem.h"

#include "api/LocalizeAPI.h"
#include "api/AdAPI.h"
#include "api/NameInputAPI.h"
#include "util/ImageLoader.h"

struct pixel {
    static pixel Default;
    pixel(Entity p=0, Entity parent=0, bool enabled=false):p(p), parent(parent), enabled(enabled) {}
    Entity p;
    Entity parent;
    Vector2 finalPosition;
    Color finalColor;
    Vector2 finalSize;
    bool enabled;
    bool operator==(pixel a){
    if (p == a.p && parent == a.parent)
        return true;
    return false;
    }
};

inline std::ostream& operator<<(std::ostream& s, const pixel& v){
    s << "p = " << v.p << ", parent = " << v.parent << std::endl;
    s << "finalPosition = " << v.finalPosition << std::endl;
    s << "finalColor = " << v.finalColor << std::endl;
    s << "finalSize = " << v.finalSize << std::endl;
    s << "enabled = " << v.enabled << std::endl;
    return s;
}

class PixelManager {
    public:
        static const int DIVIDE_BY;
        static const float SPEED;

        PixelManager(std::string assetName, AssetAPI* assetAPI);
        virtual ~PixelManager();

        pixel& findPixel(Vector2 pos);

        void updatePixel();

        bool changeBackGround(std::string assetName);

        void clickedOn(Vector2 position);
    private:
        bool splitPixel(pixel& p);

        bool fusePixel(pixel& p);

        Color averageColor(Vector2 size, Vector2 position);

        ImageDesc bg;
        
        std::list<pixel> pixels;

        AssetAPI* asset;
};

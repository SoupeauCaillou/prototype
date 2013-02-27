#include "PixelManager.h"

#include <sstream>

#include <base/Log.h>
#include <base/TouchInputManager.h>
#include <base/MathUtil.h>
#include <base/EntityManager.h>
#include <base/TimeUtil.h>
#include <base/PlacementHelper.h>
#include "util/IntersectionUtil.h"
#include "api/AssetAPI.h"

#include "api/NameInputAPI.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/ADSRSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/TaskAISystem.h"
#include "systems/MusicSystem.h"
#include "systems/ContainerSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/ScrollingSystem.h"
#include "systems/MorphingSystem.h"

#include <cmath>

const float PixelManager::SPEED = 0.05f;
const int PixelManager::DIVIDE_BY = 4;

pixel pixel::Default;

PixelManager::PixelManager(std::string assetName, AssetAPI* assetAPI)
{
    asset = assetAPI;
    changeBackGround(assetName);

    theRenderingSystem.loadAtlas("cercle", true);
    
    pixel newPixel = pixel(theEntityManager.CreateEntity());
    ADD_COMPONENT(newPixel.p, Transformation);
    TRANSFORM(newPixel.p)->z = 0.1;
    TRANSFORM(newPixel.p)->size = Vector2(theRenderingSystem.screenW, theRenderingSystem.screenH);
    TRANSFORM(newPixel.p)->position = Vector2(0,0);
    ADD_COMPONENT(newPixel.p, Rendering);

    newPixel.finalPosition = TRANSFORM(newPixel.p)->position;
    newPixel.finalSize = TRANSFORM(newPixel.p)->size;

    if (bg.datas != 0)
        newPixel.finalColor = averageColor(TRANSFORM(newPixel.p)->size, TRANSFORM(newPixel.p)->position);
    else
        newPixel.finalColor = Color::random();

    //~ RENDERING(newPixel.p)->texture = theRenderingSystem.loadTextureFile("cercle");
    //RENDERING(newPixel.p)->texture = theRenderingSystem.loadTextureFile("carre_arrondi");
    RENDERING(newPixel.p)->color = newPixel.finalColor;
    RENDERING(newPixel.p)->hide = false;
    pixels.push_back(newPixel);
}

PixelManager::~PixelManager(){}

pixel& PixelManager::findPixel(Vector2 pos)
{
    for (std::list<pixel>::iterator it = pixels.begin(); it!=pixels.end(); ++it)
    {
        if (RENDERING(it->p)->hide)
            continue;

        if (IntersectionUtil::pointRectangle(pos, TRANSFORM(it->p)->position, TRANSFORM(it->p)->size))
        {
            return *it;
        }
    }
    return pixel::Default;
}

void PixelManager::updatePixel()
{
    if (MathUtil::RandomIntInRange(0, 20) < 1)
    {
        splitPixel(findPixel(Vector2(MathUtil::RandomFloatInRange(0, theRenderingSystem.screenW) - theRenderingSystem.screenW/2 ,
                          MathUtil::RandomFloatInRange(0, theRenderingSystem.screenH) - theRenderingSystem.screenH/2)));
    }
    if (MathUtil::RandomIntInRange(0, 20) < 1)
    {
        fusePixel(findPixel(Vector2(MathUtil::RandomFloatInRange(0, theRenderingSystem.screenW) - theRenderingSystem.screenW/2 ,
                          MathUtil::RandomFloatInRange(0, theRenderingSystem.screenH) - theRenderingSystem.screenH/2)));
    }
    
    LOGI("[Update operation] Number of pixel : %lu", pixels.size());
    for (std::list<pixel>::iterator it = pixels.begin(); it != pixels.end(); ++it)
    {
        if(it->enabled || RENDERING(it->p)->hide)
            continue;
        if (TRANSFORM(it->p)->position != it->finalPosition)
        {
            Vector2 t = it->finalPosition - TRANSFORM(it->p)->position;
            Vector2 step = Vector2().Normalize(t) * SPEED;

            Color c = RENDERING(it->p)->color;

            RENDERING(it->p)->color.r += (it->finalColor.r - c.r) * (step.X / t.X);
            RENDERING(it->p)->color.g += (it->finalColor.g - c.g) * (step.X / t.X);
            RENDERING(it->p)->color.b += (it->finalColor.b - c.b) * (step.X / t.X);

            TRANSFORM(it->p)->size +=  (it->finalSize - TRANSFORM(it->p)->size) * (step.X / t.X);
            TRANSFORM(it->p)->position += step;

            it->enabled = false;
            if (Vector2::Distance(it->finalPosition, TRANSFORM(it->p)->position) < 0.05)
            {
                TRANSFORM(it->p)->position = it->finalPosition;
                RENDERING(it->p)->color = it->finalColor;
                TRANSFORM(it->p)->size = it->finalSize;
                it->enabled = true;
            }
        }
        else
        {
            it->enabled = true;
        }
    }
}

bool PixelManager::changeBackGround(std::string assetName)
{
    // Load an image :
    FileBuffer file = asset->loadAsset(assetName);

    if (file.data) {
        bg = ImageLoader::loadPng(assetName, file);
        if(bg.datas != 0)
            return true;
    }

    return false;
}

bool PixelManager::splitPixel(pixel& p)
{
    if (p.enabled && 
        !(TRANSFORM(p.p)->size < 
        Vector2(theRenderingSystem.screenW, theRenderingSystem.screenH) * 0.02))
    {
        bool children = false;
        for (std::list<pixel>::iterator it = pixels.begin(); it != pixels.end(); ++it)
        {
            if (it->parent == p.p)
            {
                children = true;
                TRANSFORM(it->p)->size = TRANSFORM(it->parent)->size;
                TRANSFORM(it->p)->position = TRANSFORM(it->parent)->position;
                RENDERING(it->p)->color = RENDERING(it->parent)->color;
                RENDERING(it->p)->hide = false;
                it->enabled = false;
            }
        }

        if (!children)
        {
            for (int i=0; i<DIVIDE_BY; ++i)
            {
                pixel newPixel = pixel(theEntityManager.CreateEntity(), p.p, false);
                ADD_COMPONENT(newPixel.p, Transformation);
                TRANSFORM(newPixel.p)->z = MathUtil::RandomFloatInRange(0.1,1);
                TRANSFORM(newPixel.p)->size = TRANSFORM(newPixel.parent)->size;
                TRANSFORM(newPixel.p)->position = TRANSFORM(newPixel.parent)->position;
                ADD_COMPONENT(newPixel.p, Rendering);
                //~ RENDERING(newPixel.p)->texture = theRenderingSystem.loadTextureFile("cercle");
                //~ RENDERING(newPixel.p)->texture = theRenderingSystem.loadTextureFile("carre_arrondi");
                RENDERING(newPixel.p)->color = RENDERING(newPixel.parent)->color;
                RENDERING(newPixel.p)->hide = false;

                newPixel.finalPosition = Vector2(TRANSFORM(newPixel.parent)->position.X + TRANSFORM(newPixel.parent)->size.X/4 * (-1+2*(i%2)),
                                             TRANSFORM(newPixel.parent)->position.Y + TRANSFORM(newPixel.parent)->size.Y/4 * (-1+2*(i/2)));
                newPixel.finalSize = TRANSFORM(newPixel.parent)->size / 2;
                if (bg.datas != 0)
                    newPixel.finalColor = averageColor(newPixel.finalSize, newPixel.finalPosition);
                else
                    newPixel.finalColor = Color::random();
                newPixel.enabled = false;
                pixels.push_back(newPixel);
            }
        }
        RENDERING(p.p)->hide = true;
    }

    return p.enabled;
}

bool PixelManager::fusePixel(pixel& p)
{
    if (p.enabled && p.parent != 0)
    {
        pixel *parent = 0;
        std::vector<std::list<pixel>::iterator> children;

        for (std::list<pixel>::iterator it = pixels.begin(); it != pixels.end(); ++it)
        {
            
            if (it->p == p.parent)
            {
                parent = &(*it);
            }
            if (p.parent == it->parent && RENDERING(it->p)->hide == false && it->enabled)
            {
                children.push_back(it);
            }
        }

        LOGI("[Fuse operation] Number of children found : %lu", children.size());
        if (children.size() != 4)
        {
            return false;
        }
        
        LOGI("[Fuse operation] Parent entity : %lu", parent->p);
        TRANSFORM(parent->p)->size = TRANSFORM(p.p)->size;
        TRANSFORM(parent->p)->position = TRANSFORM(p.p)->position;
        RENDERING(parent->p)->color = RENDERING(p.p)->color;
        RENDERING(parent->p)->hide = false;
        parent->enabled = false;

        for (unsigned long i=0; i<children.size(); ++i)
        {
            LOGI("[Fuse operation] Children to hide : %lu", children[i]->p);
            RENDERING(children[i]->p)->hide = true;
            children[i]->enabled = false;
        }

        children.clear();

        return true;
    }
    return false;
}

void PixelManager::clickedOn(Vector2 position)
{
    splitPixel(findPixel(position));
    //~ fusePixel(findPixel(position));
}

Color PixelManager::averageColor(Vector2 size, Vector2 position)
{
    uint moyR=0, moyG=0, moyB=0;

    Vector2 sizeInPixel = size;
    sizeInPixel.X = int(bg.width * (sizeInPixel.X /theRenderingSystem.screenW));
    sizeInPixel.Y = int(bg.height * (sizeInPixel.Y /theRenderingSystem.screenH));

    Vector2 pos = (position + (Vector2(theRenderingSystem.screenW, theRenderingSystem.screenH) - size) / 2) / Vector2(theRenderingSystem.screenW, theRenderingSystem.screenH);
    pos *= Vector2(bg.width, bg.height);
    pos.X = int(pos.X);
    pos.Y = int(pos.Y);

    for (int i=pos.X*bg.channels; i<(pos.X + sizeInPixel.X)*bg.channels; i+=bg.channels)
    {
        for (int j=(bg.height - 1 - pos.Y)*bg.channels; j>((bg.height -1 - pos.Y) - sizeInPixel.Y)*bg.channels; j-=bg.channels)
        {
            moyR += bg.datas[i + j*bg.width] & 0xFF;
            moyG += bg.datas[i + j*bg.width + 1] & 0xFF;
            moyB += bg.datas[i + j*bg.width + 2] & 0xFF;
        }
    }

    moyR /= (sizeInPixel.X * sizeInPixel.Y);
    moyG /= (sizeInPixel.X * sizeInPixel.Y);
    moyB /= (sizeInPixel.X * sizeInPixel.Y);

    return Color(moyR/256.,moyG/256.,moyB/256.);
}

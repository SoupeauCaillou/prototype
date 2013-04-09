#include "PixelManager.h"

#include <sstream>

#include <glm/gtc/random.hpp>
#include "systems/TransformationSystem.h"
#include "util/IntersectionUtil.h"
#include "PrototypeGame.h"

const float PixelManager::SPEED = 0.05f;
const int PixelManager::DIVIDE_BY = 4;

pixel pixel::Default;

PixelManager::PixelManager(std::string assetName, AssetAPI *assetapi)
{
    assetAPI = assetapi;
    changeBackGround(assetName);

    // theRenderingSystem.loadAtlas("cercle", true);
    
    pixel newPixel = pixel(theEntityManager.CreateEntity());
    ADD_COMPONENT(newPixel.p, Transformation);
    TRANSFORM(newPixel.p)->z = 0.1;
    TRANSFORM(newPixel.p)->size = glm::vec2(float(theRenderingSystem.screenW), float(theRenderingSystem.screenH));
    TRANSFORM(newPixel.p)->position = glm::vec2(0.0f);
    ADD_COMPONENT(newPixel.p, Rendering);

    newPixel.finalPosition = TRANSFORM(newPixel.p)->position;
    newPixel.finalSize = TRANSFORM(newPixel.p)->size;

    if (bg.datas != 0)
        newPixel.finalColor = averageColor(TRANSFORM(newPixel.p)->size, TRANSFORM(newPixel.p)->position);
    else
        newPixel.finalColor = Color::random();

    // RENDERING(newPixel.p)->texture = theRenderingSystem.loadTextureFile("cercle");
    // RENDERING(newPixel.p)->texture = theRenderingSystem.loadTextureFile("carre_arrondi");
    RENDERING(newPixel.p)->color = newPixel.finalColor;
    RENDERING(newPixel.p)->show = true;
    pixels.push_back(newPixel);
}

PixelManager::~PixelManager(){}

pixel& PixelManager::findPixel(glm::vec2 pos)
{
    for (auto& pixel : pixels) {
        if (!RENDERING(pixel.p)->show)
            continue;

        if (IntersectionUtil::pointRectangle(pos, TRANSFORM(pixel.p)->position, TRANSFORM(pixel.p)->size))
        {
            return pixel;
        }
    }
    return pixel::Default;
}

void PixelManager::updatePixel()
{
    if (glm::linearRand(0.0f, 10.0f) < 1.0f)
    {
        splitPixel(findPixel(glm::vec2(glm::linearRand(0.0f, (float)theRenderingSystem.screenW) - theRenderingSystem.screenW/2 ,
                                       glm::linearRand(0.0f, (float)theRenderingSystem.screenH) - theRenderingSystem.screenH/2)));
    }
    if (glm::linearRand(0.0f, 10.0f) < 1.0f)
    {
        fusePixel(findPixel(glm::vec2(glm::linearRand(0.0f, (float)theRenderingSystem.screenW) - theRenderingSystem.screenW/2 ,
                                      glm::linearRand(0.0f, (float)theRenderingSystem.screenH) - theRenderingSystem.screenH/2)));
    }
    
    for (auto& pixel : pixels)
    {
        if(pixel.enabled || !RENDERING(pixel.p)->show)
            continue;
        if (TRANSFORM(pixel.p)->position.x != pixel.finalPosition.x &&
            TRANSFORM(pixel.p)->position.y != pixel.finalPosition.y) {
            glm::vec2 t = pixel.finalPosition - TRANSFORM(pixel.p)->position;
            glm::vec2 step = glm::normalize(t) * SPEED;

            Color c = RENDERING(pixel.p)->color;

            RENDERING(pixel.p)->color.r += (pixel.finalColor.r - c.r) * (step.x / t.x);
            RENDERING(pixel.p)->color.g += (pixel.finalColor.g - c.g) * (step.x / t.x);
            RENDERING(pixel.p)->color.b += (pixel.finalColor.b - c.b) * (step.x / t.x);

            TRANSFORM(pixel.p)->size +=  (pixel.finalSize - TRANSFORM(pixel.p)->size) * (float)(step.x / t.x);
            TRANSFORM(pixel.p)->position += step;

            pixel.enabled = false;
            if (glm::distance(pixel.finalPosition, TRANSFORM(pixel.p)->position) < 0.05f)
            {
                TRANSFORM(pixel.p)->position = pixel.finalPosition;
                RENDERING(pixel.p)->color = pixel.finalColor;
                TRANSFORM(pixel.p)->size = pixel.finalSize;
                pixel.enabled = true;
            }
        }
        else
        {
            pixel.enabled = true;
        }
    }
}

bool PixelManager::changeBackGround(std::string assetName)
{
    // Load an image :
    FileBuffer file = assetAPI->loadAsset(assetName);

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
        (TRANSFORM(p.p)->size.x > theRenderingSystem.screenW * 0.02f)) {
        bool children = false;
        for (auto& pixel : pixels) {
            if (TRANSFORM(pixel.p)->parent == p.p) {
                children = true;
                TRANSFORM(pixel.p)->size = TRANSFORM(p.p)->size;
                TRANSFORM(pixel.p)->position = glm::vec2(0.0f);
                RENDERING(pixel.p)->color = RENDERING(p.p)->color;
                RENDERING(pixel.p)->show = true;
                pixel.enabled = false;
            }
        }

        if (!children)
        {
            for (int i=0; i<DIVIDE_BY; ++i)
            {
                pixel newPixel = pixel(theEntityManager.CreateEntity(), false);
                ADD_COMPONENT(newPixel.p, Transformation);
                TRANSFORM(newPixel.p)->parent = p.p;
                TRANSFORM(newPixel.p)->z = i * 0.01f;
                TRANSFORM(newPixel.p)->size = TRANSFORM(p.p)->size;
                ADD_COMPONENT(newPixel.p, Rendering);
                //~ RENDERING(newPixel.p)->texture = theRenderingSystem.loadTextureFile("cercle");
                //~ RENDERING(newPixel.p)->texture = theRenderingSystem.loadTextureFile("carre_arrondi");
                RENDERING(newPixel.p)->color = RENDERING(p.p)->color;
                RENDERING(newPixel.p)->show = true;

                newPixel.finalPosition = glm::vec2(TRANSFORM(p.p)->size.x/4.0f * (float)(-1+2*(i%2)),
                                                   TRANSFORM(p.p)->size.y/4.0f * (float)(-1+2*(i/2)));
                newPixel.finalSize = TRANSFORM(p.p)->size / 2.0f;
                if (bg.datas != 0)
                    newPixel.finalColor = averageColor(newPixel.finalSize, TRANSFORM(p.p)->worldPosition + newPixel.finalPosition);
                else
                    newPixel.finalColor = Color::random();
                newPixel.enabled = false;
                pixels.push_back(newPixel);
            }
        }
        RENDERING(p.p)->show = false;
    }

    return p.enabled;
}

bool PixelManager::fusePixel(pixel& p)
{
    if (p.enabled && TRANSFORM(p.p)->parent != 0)
    {
        std::list<pixel>::iterator parent;
        std::vector<std::list<pixel>::iterator> children;

        for (std::list<pixel>::iterator it = pixels.begin(); it != pixels.end(); ++it)
        {
            
            if (it->p == TRANSFORM(p.p)->parent)
            {
                parent = it;
            }
            if (TRANSFORM(p.p)->parent == TRANSFORM(it->p)->parent && RENDERING(it->p)->show && it->enabled)
            {
                children.push_back(it);
            }
        }

        if (children.size() != 4)
        {
            return false;
        }

        TRANSFORM(parent->p)->size = TRANSFORM(p.p)->size;
        TRANSFORM(parent->p)->position = TRANSFORM(p.p)->position;
        RENDERING(parent->p)->color = RENDERING(p.p)->color;
        RENDERING(parent->p)->show = true;
        parent->enabled = false;

        for (unsigned long i=0; i<children.size(); ++i)
        {
            RENDERING(children[i]->p)->show = false;
            children[i]->enabled = false;
        }

        children.clear();

        return true;
    }
    return false;
}

void PixelManager::clickedOn(glm::vec2 position)
{
    splitPixel(findPixel(position));
    //~ fusePixel(findPixel(position));
}

Color PixelManager::averageColor(glm::vec2 size, glm::vec2 position)
{
    uint moyR=0, moyG=0, moyB=0;

    // convert size in index of table (int)
    std::pair<int, int> sizeInPixel = std::make_pair(bg.width * (size.x / theRenderingSystem.screenW), 
                                                    bg.height * (size.y /theRenderingSystem.screenH));
    // Convert position as size
    std::pair<int, int> pos = std::make_pair(bg.width * (position.x + (theRenderingSystem.screenW - size.x) / 2) / theRenderingSystem.screenW, 
         bg.height * (position.y + (theRenderingSystem.screenH - size.y) / 2) / theRenderingSystem.screenH);

    // Compute the average color in good area of image
    for (int i = pos.first*bg.channels; 
             i < (pos.first + sizeInPixel.first)*bg.channels; 
             i += bg.channels) {
        for (int j = (bg.height - 1 - pos.second)*bg.channels; 
                 j > ((bg.height -1 - pos.second) - sizeInPixel.second)*bg.channels; 
                 j -= bg.channels) {
            moyR += bg.datas[i + j*bg.width] & 0xFF;
            moyG += bg.datas[i + j*bg.width + 1] & 0xFF;
            moyB += bg.datas[i + j*bg.width + 2] & 0xFF;
        }
    }

    moyR /= (sizeInPixel.first * sizeInPixel.second);
    moyG /= (sizeInPixel.first * sizeInPixel.second);
    moyB /= (sizeInPixel.first * sizeInPixel.second);

    return Color(moyR/256.,moyG/256.,moyB/256.);
}

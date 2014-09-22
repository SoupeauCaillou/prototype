#include "LevelLoader.h"

#include "systems/AutonomousAgentSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "base/PlacementHelper.h"
#include "base/NamedAssetLibrary.h"

#include "util/DataFileParser.h"
#include "util/MurmurHash.h"

#include <fstream>

static Entity createEntity(const DataFileParser & dfp, int number, const std::string & section) {
    std::stringstream ss;

    Entity e = theEntityManager.CreateEntityFromTemplate(  ("game/" + section).c_str() );

    hash_t sec_h = Murmur::RuntimeHash(section.c_str());

    // Entity are defined either as (position, size, rotation) or (polygon)
    ss << section << "_position_" << number;
    if (! dfp.get(sec_h, Murmur::RuntimeHash(ss.str().c_str()), &TRANSFORM(e)->position.x, 2, false)) {
        ss.str(""); ss << section << "_position_" << number;

        if (!dfp.get(sec_h, Murmur::RuntimeHash(ss.str().c_str()), &TRANSFORM(e)->position.x, 2)) {
            goto polygonMode;
        }
        TRANSFORM(e)->position = PlacementHelper::GimpPositionToScreen(TRANSFORM(e)->position);
    }


    ss.str(""); ss << section << "_size_" << number;
    if (! dfp.get(sec_h, Murmur::RuntimeHash(ss.str().c_str()), &TRANSFORM(e)->size.x, 2, false)) {
        ss.str(""); ss << section << "_size%gimp_" << number;
        dfp.get(sec_h, Murmur::RuntimeHash(ss.str().c_str()), &TRANSFORM(e)->size.x, 2);
        TRANSFORM(e)->size = PlacementHelper::GimpSizeToScreen(TRANSFORM(e)->size);
    }

    ss.str(""); ss << section << "_rotation_" << number;
    dfp.get(sec_h, Murmur::RuntimeHash(ss.str().c_str()), &TRANSFORM(e)->rotation, 1);
    TRANSFORM(e)->rotation = glm::radians(TRANSFORM(e)->rotation);

    LOGV(1, "One more '" << section << "' at pos:" << TRANSFORM(e)->position
         << " and size " << TRANSFORM(e)->size);

    return e;

polygonMode:
    ss.str("");
    ss.clear();
    ss << section << "_polygon_" << number;
    int cnt = dfp.getSubStringCount(sec_h, Murmur::RuntimeHash(ss.str().c_str()));
    LOGI(cnt << '/' << ss.str());
    if (cnt == -1) {
        LOGF("Handle without %gimp modifier");
    }
    LOGW_IF(cnt % 2, "There should be 2 * nb_point coordinate");
    int* pts = new int[cnt];
    dfp.get(sec_h, Murmur::RuntimeHash(ss.str().c_str()), pts, cnt);

    // Create a new polygon
    Polygon p;
    for (int i=0; i<cnt/2; i++) {
        p.vertices.push_back(
            PlacementHelper::GimpPositionToScreen(glm::vec2(pts[2*i], pts[2*i+1])));
    }

    // make sure vertices are defined clockwise
    for (unsigned i=0; i<p.vertices.size() - 1; ++i) {
        // find two consecutives y > 0 point
        if (p.vertices[i].y > 0 && p.vertices[i + 1].y > 0) {
            if (p.vertices[i].x < p.vertices[i + 1].x) {
                LOGI("Polygon not in clockwise order. Reversing");
                std::reverse(p.vertices.begin(), p.vertices.end());
            }
        }
    }

    // Create indice set (polygons are drawn as triangle-strip)
    for (unsigned i=1; i<p.vertices.size() - 1; i+=2) {
        p.indices.push_back(i);
        p.indices.push_back(0);
        p.indices.push_back(i + 1);
        if ((i+2) < p.vertices.size())
            p.indices.push_back(i + 2);
        // p.indices.push_back(i + 2);
    }

    // Make size act as a bouding box
    glm::vec2 x (p.vertices.front()), y(p.vertices.front());
    for (auto& v: p.vertices) {
        x.x = glm::min(x.x, v.x);
        x.y = glm::max(x.y, v.x);
        y.x = glm::min(y.x, v.y);
        y.y = glm::max(y.y, v.y);
    }
    // Fix position
    glm::vec2 center((x.x + x.y) / 2, (y.x + y.y) / 2);
    for (auto& v: p.vertices) {
        v -= center;
    }
    TRANSFORM(e)->position = center;
    // Fix center
    TRANSFORM(e)->size = glm::vec2(x.y - x.x, y.y - y.x);
    glm::vec2 invSize = 1.0f / TRANSFORM(e)->size;
    for (auto& v: p.vertices) {
        v *= invSize;
    }

    theTransformationSystem.shapes.push_back(p);
    TRANSFORM(e)->shape = (Shape::Enum) (theTransformationSystem.shapes.size() - 1);
    RENDERING(e)->texture = InvalidTextureRef;

    return e;
}

void LevelLoader::init(AssetAPI* assetAPI) {
    this->assetAPI = assetAPI;
}

void LevelLoader::load(FileBuffer & fb) {
    DataFileParser dfp;
    dfp.load(fb, "level_loader");

    //////////////////////////////////////////////////
    LOGT("cleaner way of removing previous level");
    sheep.clear();
    walls.clear();
    bushes.clear();
    zones.clear();
    ////////////////////////////////////////////////////

    dfp.get(DataFileParser::GlobalSection, HASH("objective_arrived", 0xe1afe979), &objectiveArrived, 1);
    dfp.get(DataFileParser::GlobalSection, HASH("objective_survived", 0xe0b7b21f), &objectiveSurvived, 1);
    dfp.get(DataFileParser::GlobalSection, HASH("objective_time_limit", 0xbdd44ba3), &objectiveTimeLimit, 1);
    std::string backgroundTexture;
    dfp.get(DataFileParser::GlobalSection, HASH("background_texture", 0x63faaee8), &backgroundTexture, 1);
    background = theEntityManager.CreateEntityFromTemplate("game/level_background");
    RENDERING(background)->texture = theRenderingSystem.loadTextureFile(backgroundTexture.c_str());

    // create sheep
    for (unsigned i = 1; i <= dfp.sectionSize(HASH("sheep", 0x3a3ee38b)) / 3; ++i) {
        Entity e = createEntity(dfp, i, "sheep");

        sheep.push_back(e);
    }

    auto sheep = theAutonomousAgentSystem.RetrieveAllEntityWithComponent();

    //create bushes
    for (unsigned i = 35; i <= 35; ++i) {//dfp.sectionSize("bush"); ++i) {
        Entity bush = createEntity(dfp, i, "bush");
        for (auto s : sheep) {
            AUTONOMOUS(s)->obstacles.push_back(bush);
        }
        bushes.push_back(bush);
    }

    //create walls
    for (unsigned i = 1; i <= dfp.sectionSize(HASH("wall", 0x4b3afad3)) / 3; ++i) {
        Entity wall = createEntity(dfp, i, "wall");

        for (auto s : sheep) {
            AUTONOMOUS(s)->walls.push_back(wall);
        };
        walls.push_back(wall);
    }

    //create final zone
    for (unsigned i = 1; i <= dfp.sectionSize(HASH("zone", 0xd0806862)) / 3; ++i) {
         zones.push_back(createEntity(dfp, i, "zone"));
    }
}

static void writeSection(std::ofstream & of, const std::string & name, const std::vector<Entity> & v) {
    of << "[" << name << "]" << std::endl;
    for (unsigned i = 1; i <= v.size(); i++) {
        auto tc = TRANSFORM(v[i-1]);
        of << name << "_position_" << i << "\t=" << tc->position << std::endl;
        of << name << "_size_" << i << "\t= " << tc->size << std::endl;
        of << name << "_rotation_" << i << "\t= " << glm::degrees(tc->rotation) << std::endl;
    }
    of << std::endl;
}

void LevelLoader::save(const std::string & path) {
#if SAC_DEBUG
    std::ofstream of(path);

    of << "objective_arrived\t= " << objectiveArrived << std::endl;
    of << "objective_survived\t= " << objectiveSurvived << std::endl;
    of << "objective_time_limit\t= " << objectiveTimeLimit << std::endl;
    of << "background_texture\t= " << theRenderingSystem.textureLibrary.ref2Name(RENDERING(background)->texture) << std::endl;
    writeSection(of, "sheep", sheep);
    writeSection(of, "wall", walls);
    writeSection(of, "bush", bushes);
    writeSection(of, "zone", zones);
#else
    LOGE("Not handled in release mode!");
#endif
}

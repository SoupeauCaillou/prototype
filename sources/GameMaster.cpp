#include "GameMaster.h"

#include "base/EntityManager.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/FighterSystem.h"
#include "systems/PlayerSystem.h"
#include "systems/EquipmentSystem.h"
#include "systems/SlotSystem.h"
#include "systems/PickableSystem.h"

static const float scale = 1 / 200.0;
#define P(v, ref) ((Vector2(-0.5, 0.5) * ref + Vector2(v.X, -v.Y)) * scale)

static Entity createFighter() {
    Vector2 ref(363, 393);
    Entity e = theEntityManager.CreateEntity();
    ADD_COMPONENT(e, Transformation);
    TRANSFORM(e)->size = ref * scale;
    TRANSFORM(e)->z = 0.5;
    ADD_COMPONENT(e, Fighter);
    ADD_COMPONENT(e, Pickable);

    std::string textures[] = {"head", "torso", "left_arm", "right_arm", "left_leg", "right_leg"};
    Vector2 positions[] = {
        Vector2(196, 65), Vector2(193, 181), Vector2(84, 168), Vector2(294, 162), Vector2(142, 315), Vector2(235, 316)
    };
    Vector2 anchors[] = {
        Vector2(56, 44), Vector2(67, 74), Vector2(27, 29), Vector2(74, 20), Vector2::Zero, Vector2::Zero
    };
    for (int i=0; i<6; i++) {
        Entity member = FIGHTER(e)->members[i] = theEntityManager.CreateEntity();
        ADD_COMPONENT(member, Transformation);
        TRANSFORM(member)->parent = e;
        TRANSFORM(member)->size = theRenderingSystem.getTextureSize(textures[i]) * scale;
        TRANSFORM(member)->position = P(positions[i], ref);
        ADD_COMPONENT(member, Rendering);
        RENDERING(member)->texture = theRenderingSystem.loadTextureFile(textures[i]);
        RENDERING(member)->hide = false;
        ADD_COMPONENT(member, Slot);
        SLOT(member)->anchor = P(anchors[i], theRenderingSystem.getTextureSize(textures[i]));
    }
    return e;
}

static Entity createEquipment() {
    std::string textures[] = {"sword1", "doubleaxe1", "shield1", "helmet1", "armor1", "bow"};
    Vector2 anchors[] = {
        Vector2(36, 158), Vector2(153, 122), Vector2(67, 52), Vector2(118, 41), Vector2(64, 63), Vector2(75, 148)
    };
    int type = MathUtil::RandomInt((int)EquipmentType::Count);
    Entity eq = theEntityManager.CreateEntity();
    ADD_COMPONENT(eq, Transformation);
    TRANSFORM(eq)->z = 0.1;
    TRANSFORM(eq)->size = theRenderingSystem.getTextureSize(textures[type]) * scale;
    ADD_COMPONENT(eq, Rendering);
    RENDERING(eq)->texture = theRenderingSystem.loadTextureFile(textures[type]);
    RENDERING(eq)->hide = false;
    ADD_COMPONENT(eq, Equipment);
    EQUIPMENT(eq)->type = (EquipmentType::Enum)type;
    EQUIPMENT(eq)->anchor = P(anchors[type], theRenderingSystem.getTextureSize(textures[type]));
    ADD_COMPONENT(eq, Pickable);
    return eq;
}

void GameMaster::stateChanged(State::Enum oldState, State::Enum newState) {
    if (oldState == State::Menu && newState == State::Transition) {
        // Create 2 players entity
        Entity p1 = theEntityManager.CreateEntity();
        ADD_COMPONENT(p1, Player);
        Entity p2 = theEntityManager.CreateEntity();
        ADD_COMPONENT(p2, Player);

        // Create 12 x 2 fighters
        for (int i=0; i<12; i++) {
            Entity f1 = createFighter();
            FIGHTER(f1)->player = p1;
            Entity f2 = createFighter();
            FIGHTER(f2)->player = p2;
        }
        // Create random equipments
        for (int i=0; i<45; i++) {
            createEquipment();
        }
    }
}

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

#include "Scenes.h"

#include "PrototypeGame.h"

#include "systems/AutonomousAgentSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/AnchorSystem.h"

#include "base/TouchInputManager.h"
#include "base/StateMachine.h"

#include "util/IntersectionUtil.h"

#include "api/KeyboardInputHandlerAPI.h"
#include <glm/gtx/vector_angle.hpp>

namespace Mode {
    enum Enum {
        None,
        Grab,
        Scale,
        ScaleX,
        ScaleY,
        Rotate
    };
}

namespace Type {
    enum Enum {
        Sheep,
        Wall,
        Bush,
        Zone,
        Cursor
    };
}
struct EditorScene : public StateHandler<Scene::Enum> {
    PrototypeGame* game;
    std::vector<Entity> modifiable;
    Entity selected;
    Entity highlight;
    Mode::Enum mode;
    glm::vec2 modeStartPosition;

    struct {
        glm::vec2 size;
        float rotation;
    } initial;

    EditorScene(PrototypeGame* game) : StateHandler<Scene::Enum>() {
        this->game = game;
    }

    void setup() {
        highlight = addEntity(Type::Cursor);
        ADD_COMPONENT(highlight, Anchor);
        ANCHOR(highlight)->z = -0.02;
        RENDERING(highlight)->show = false;
    }



    ///----------------------------------------------------------------------------//
    ///--------------------- ENTER SECTION ----------------------------------------//
    ///----------------------------------------------------------------------------//

    void onEnter(Scene::Enum) override {
        selected = 0;
        mode = Mode::None;

        thePhysicsSystem.forEachECDo([] (Entity, PhysicsComponent* pc) -> void {
            pc->mass = 0;
        });
    }

    void updateSelection(Entity newSelection, float ) {
        if (newSelection == 0) {
            RENDERING(highlight)->show = false;
            ANCHOR(highlight)->parent = 0;
        } else {
            RENDERING(highlight)->show = true;
            ANCHOR(highlight)->parent = newSelection;
            TRANSFORM(highlight)->size = TRANSFORM(newSelection)->size * 1.05f;
        }
        selected = newSelection;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- UPDATE SECTION ---------------------------------------//
    ///----------------------------------------------------------------------------//
    void updateModifiableList() {
        modifiable.clear();

        std::vector<Entity> entities = theEntityManager.allEntities();
        for (Entity e: entities) {
            std::string name = theEntityManager.entityName(e);
            if (name.find("sheep") != std::string::npos ||
                name.find("wall") != std::string::npos ||
                name.find("bush") != std::string::npos ||
                name.find("zone") != std::string::npos) {
                modifiable.push_back(e);
            }
        }
    }

    static std::string typeToName(Type::Enum t) {
        switch (t) {
            case Type::Sheep: return "sheep";
            case Type::Wall: return "wall";
            case Type::Bush: return "bush";
            case Type::Zone: return "zone";
            case Type::Cursor: return "cursor";
        }
        return "";
    }

    static Color typeToColor(Type::Enum t) {
        switch (t) {
            case Type::Sheep: return Color(1, 1, 1);
            case Type::Wall: return Color(0, 0, 0);
            case Type::Bush: return Color(0, 1, 0);
            case Type::Zone: return Color(0, 0, 1);
            case Type::Cursor: return Color(0.2, 0.2, 0.2, 0.75);
        }
        return Color();
    }

    Entity addEntity(Type::Enum t) {
        Entity e = theEntityManager.CreateEntity(typeToName(t));
        ADD_COMPONENT(e, Rendering);
        RENDERING(e)->show = 1;
        RENDERING(e)->color = typeToColor(t);
        ADD_COMPONENT(e, Transformation);
        TRANSFORM(e)->z = 0.5;
        TRANSFORM(e)->size = glm::vec2(0.5);
        return e;
    }

    Scene::Enum update(float dt) override {
        const glm::vec2 cursorPos = theTouchInputManager.getTouchLastPosition();

        if (selected) {
            if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyReleased(42)) {
                LOGI("Mode GRAB");
                mode = Mode::Grab;
            }
            // scale
            else if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyReleased(39)) {
                LOGI("Mode SCALE");
                mode = Mode::Scale;
            }
            // rotate
            else if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyReleased(27)) {
                LOGI("Mode ROTATE");
                mode = Mode::Rotate;
            }
            // none
            else if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyReleased(36)) {
                LOGI("Mode NONE");
                mode = Mode::None;
            }
            if (mode == Mode::Scale) {
                if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyReleased(53)) {
                    mode = Mode::ScaleX;
                }
                if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyReleased(29)) {
                    mode = Mode::ScaleY;
                }
            }


            if (theTouchInputManager.isTouched()) {
                switch (mode) {
                    case Mode::None: {
                        break;
                    }
                    case Mode::Grab: {
                        TRANSFORM(selected)->position = cursorPos;
                        break;
                    }
                    case Mode::Scale:
                    case Mode::ScaleX:
                    case Mode::ScaleY: {
                        if (!theTouchInputManager.wasTouched()) {
                            modeStartPosition = cursorPos;
                            initial.size = TRANSFORM(selected)->size;
                        } else {
                            float scale = glm::distance(cursorPos, TRANSFORM(selected)->position);

                            TRANSFORM(selected)->size = initial.size;
                            if (mode == Mode::Scale)
                                TRANSFORM(selected)->size = initial.size * scale;
                            else if (mode == Mode::ScaleX) 
                                TRANSFORM(selected)->size.x = initial.size.x * scale;
                            else
                                TRANSFORM(selected)->size.y = initial.size.y * scale;
                        }
                        break;
                    }
                    case Mode::Rotate: {
                        if (!theTouchInputManager.wasTouched()) {
                            modeStartPosition = cursorPos;
                            initial.rotation = TRANSFORM(selected)->rotation;
                            LOGI("RESET ROT");
                        } else {
                            glm::vec2 diff(cursorPos - TRANSFORM(selected)->position);
                            if (glm::length(diff) > 1) {
                                float rotation = glm::orientedAngle(glm::vec2(1, 0), glm::normalize(diff));
                                TRANSFORM(selected)->rotation = initial.rotation + rotation;
                            }
                        }
                        break;
                    }
                }
            }
        }

        if (theTouchInputManager.hasClicked(1)) {
            updateModifiableList();

            auto cursorPos2 = theTouchInputManager.getTouchLastPosition(1);
            LOGI(modifiable.size() << " entities to select");
            bool useful = false;
            for (Entity e: modifiable) {
                if (IntersectionUtil::pointRectangle(cursorPos2, TRANSFORM(e))) {
                    if (e == selected) {
                        selected = 0;
                    } else {
                        selected = e;
                    }
                    useful = true;
                    mode = Mode::None;
                    break;
                }
            }
            if (!useful) {
                if (selected) {
                    selected = 0;
                    mode = Mode::None;
                } else {
                    LOGW("Useless click:" << cursorPos << "/" << cursorPos2);
                }
            }
        }

        // entity spawn
        if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyReleased(10)) {
            // add sheep
            addEntity(Type::Sheep);
        }
        else if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyReleased(11)) {
            addEntity(Type::Wall);
        }
        // rotate
        else if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyReleased(12)) {
            addEntity(Type::Bush);
        }
        // none
        else if (game->gameThreadContext->keyboardInputHandlerAPI->isKeyReleased(13)) {
            addEntity(Type::Zone);
        }

        updateSelection(selected, dt);

        return Scene::Editor;
    }


    ///----------------------------------------------------------------------------//
    ///--------------------- EXIT SECTION -----------------------------------------//
    ///----------------------------------------------------------------------------//
    void onPreExit(Scene::Enum) override {
    }

    void onExit(Scene::Enum) override {
        
    }
};

namespace Scene {
    StateHandler<Scene::Enum>* CreateEditorSceneHandler(PrototypeGame* game) {
        return new EditorScene(game);
    }
}

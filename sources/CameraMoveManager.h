/*
    This file is part of Dogtag.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Dogtag is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Dogtag is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dogtag.  If not, see <http://www.gnu.org/licenses/>.
*/


#pragma once

#include "base/Entity.h"
#include "systems/AnchorSystem.h"

namespace CameraMode {
    enum Enum {
        None,
        Pan,
        Zoom,
        Chase
    };
}

class CameraMoveManager {
public:

    void init(Entity camera, const glm::vec2& maxCamSize, float maxZoom, const glm::vec4& boundaries);

    bool update(float dt, bool inputEnabled = true);

    bool moveTowardResetPositionAndZoom(float dt);

    void reset();

    void centerOn(const glm::vec2& position);

    void setZoom(float value);

    bool isAtLimit(Cardinal::Enum e, float epsilon = 0.001);

private:
    void updateCamera(const glm::vec2 & speed, float dt);

    Entity camera;
    CameraMode::Enum mode;

    glm::vec2 maxCameraSize;
    float maxZoom;
    glm::vec4 boundaries;

    glm::vec2 zoomOriginal, panPreviousPosition;
    glm::vec2 speed;
    float zoomValue, originalZ;

    glm::vec2 chaseTarget;
};

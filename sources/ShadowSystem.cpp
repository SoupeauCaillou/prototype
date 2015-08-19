/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ShadowSystem.h"

#include "systems/AnchorSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "util/Tuning.h"

INSTANCE_IMPL(ShadowSystem);

ShadowSystem::ShadowSystem() : ComponentSystemImpl<ShadowComponent>(HASH("Shadow", 0x1bf95fef)) {}

void ShadowSystem::DoUpdate(float) {
    FOR_EACH_ENTITY_COMPONENT(Shadow, e, comp)
    Entity parent = ANCHOR(e)->parent;
    glm::vec2 parentSize = TRANSFORM(parent)->size;
    TextureRef texture = RENDERING(parent)->texture;
    if (texture != InvalidTextureRef) {
        auto* info = theRenderingSystem.textureLibrary.get(texture, false);
        if (info) {
            parentSize *= info->reduxSize;
        }
    }
    TRANSFORM(e)->size.x = theTuning.f(HASH("shadow_percent", 0x53fbfdd4)) * parentSize.x;
}
}

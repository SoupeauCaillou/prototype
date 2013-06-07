#include "CameraMoveManager.h"

#include "base/TouchInputManager.h"

#include "systems/TransformationSystem.h"

#include "glm/glm.hpp"

CameraMoveManager* CameraMoveManager::instance = 0;

CameraMoveManager* CameraMoveManager::Instance() {
	if (instance == 0) instance = new CameraMoveManager();
	return instance;
}

bool CameraMoveManager::update(float, Entity camera) {
	static glm::vec2 lastTouched = glm::vec2(0.f);
	if (theTouchInputManager.isTouched(0) && theTouchInputManager.isTouched(1)) {
        auto tc = TRANSFORM(camera);
		glm::vec2 position = theTouchInputManager.getTouchLastPosition();
		if (lastTouched == glm::vec2(0.f)) {
			lastTouched = position;
		}
		else {
			if (position != lastTouched) {
				glm::vec2 dir = position - lastTouched;
				tc->position -= dir/2.f;
				lastTouched = position;
			}
		}

        if (glm::abs(tc->position.x) + tc->size.x / 2.f > 20) {
            float posx = 20.f - tc->size.x/2.f;
            if (tc->position.x < 0)
                tc->position.x = - posx;
            else
                tc->position.x = posx;
        }

        if (glm::abs(tc->position.y) + tc->size.y / 2.f > 12.5) {

            float posy = 12.5f - tc->size.y/2.f;
            if (tc->position.y < 0)
                tc->position.y = - posy;
            else
                tc->position.y = posy;
        }

        return true;
	}
	else {
		lastTouched = glm::vec2(0.f);
        return false;
	}
}

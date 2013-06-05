#include "CameraMoveManager.h"

#include "base/TouchInputManager.h"

#include "systems/TransformationSystem.h"

#include "glm/glm.hpp"

CameraMoveManager* CameraMoveManager::instance = 0;

CameraMoveManager* CameraMoveManager::Instance() {
	if (instance == 0) instance = new CameraMoveManager();
	return instance;
}

void CameraMoveManager::update(float dt, Entity camera) {
	static glm::vec2 lastTouched = glm::vec2(0.f);
	if (theTouchInputManager.isTouched(0) && theTouchInputManager.isTouched(1)) {
		glm::vec2 position = theTouchInputManager.getTouchLastPosition();
		if (lastTouched == glm::vec2(0.f)) {
			lastTouched = position;
		}
		else {
			if (position != lastTouched) {
				glm::vec2 dir = position - lastTouched;
				TRANSFORM(camera)->position += dir/2.f;
				lastTouched = position;
			}
		}
	}
	else {
		lastTouched = glm::vec2(0.f);
	}
}
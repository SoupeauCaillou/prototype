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


#include "CameraMoveManager.h"

#include "base/TouchInputManager.h"
#include "steering/SteeringBehavior.h"

#include "systems/TransformationSystem.h"
#include "systems/CameraSystem.h"

#include "glm/glm.hpp"

void CameraMoveManager::init(Entity pCamera, const glm::vec2& maxCamSize, float pMaxZoom, const glm::vec4& pBoundaries) {
	camera = pCamera;
    mode = CameraMode::None;
    speed = glm::vec2(0.0f);
    zoomValue = 1;

    maxCameraSize = maxCamSize;
    maxZoom = pMaxZoom;
    boundaries = pBoundaries;
}

void CameraMoveManager::reset() {
	speed = glm::vec2(0.0f);
	mode = CameraMode::None;
}

void CameraMoveManager::addSpeed(const glm::vec2& move) {
    speed += move;
}

bool CameraMoveManager::update(float dt, bool inputEnabled) {
	auto* tc = TRANSFORM(camera);

	// Pick the right mode
	if (inputEnabled && theTouchInputManager.isTouched(0)) {
		// reset mode
		if (mode == CameraMode::Chase)
			mode = CameraMode::None;

#if SAC_MOBILE
		if (theTouchInputManager.isTouched(1)) {
			// Init zoom on 2nd finger touch
			if (!theTouchInputManager.wasTouched(1)) {
				glm::vec2 zoom[2];
				zoom[0] = theTouchInputManager.getTouchLastPosition(0);
				zoom[1] = theTouchInputManager.getTouchLastPosition(1);
				zoomOriginal = (zoom[1] - zoom[0]) / tc->size;
				LOGV(1, "zoomOriginal:" << zoomOriginal);
				originalZ = zoomValue;
			}
			mode = CameraMode::Zoom;
			LOGV(1, "CameraMode -> Zoom");
			speed = glm::vec2(0.0f);
		} else
#endif
		{
			// Init pan on finger touch
			if (theTouchInputManager.hasMoved(0)) {
				if (mode != CameraMode::Pan) {
					panPreviousPosition = CameraSystem::WorldToScreen(tc, theTouchInputManager.getTouchLastPosition(0));
					mode = CameraMode::Pan;
					LOGV(1, "CameraMode -> Pan [" << panPreviousPosition);
				}
			} else if (0){
				if (mode != CameraMode::None)
					LOGV(1, "CameraMode -> None");
				mode = CameraMode::None;
			}
		}
	} else if (mode != CameraMode::Chase) {
		if (mode != CameraMode::None)
			LOGV(1, "CameraMode -> None");
		mode = CameraMode::None;
	}

	switch (mode) {
		case CameraMode::Pan: {
			if (theTouchInputManager.hasMoved(0)) {
				const glm::vec2 screen = CameraSystem::WorldToScreen(tc, theTouchInputManager.getTouchLastPosition(0));
				glm::vec2 diff (screen - panPreviousPosition);
				speed -= (diff * tc->size / dt) * 0.1f;
				panPreviousPosition = screen;
			} else {
				speed -= speed * dt * 8.0f;
			}
			break;
		}
		case CameraMode::Zoom: {
			glm::vec2 zoom[2];
			for (int i=0; i<2; i++) {
				zoom[i] = theTouchInputManager.getTouchLastPosition(i);
			}
			// diff
			glm::vec2 diff = (zoom[1] - zoom[0]) / tc->size;
			float a = glm::length(diff) / glm::length(zoomOriginal);
			zoomValue = originalZ * a;//+ (1 - a) * 10 * dt;
		}
		case CameraMode::None: {
			speed -= speed * dt * 5.0f;
			break;
		}
		case CameraMode::Chase: {
			if (glm::length(tc->position - chaseTarget) > 0.01) {
				glm::vec2 accel = SteeringBehavior::arrive(
					tc->position, speed, chaseTarget, 30, 0.1);
				if (glm::length(accel) > 50)
					accel = glm::normalize(accel) * 50.0f;
				speed += accel;// * dt;
			} else {
				mode = CameraMode::None;
				speed = glm::vec2(0);
			}
			break;
		}
	}

#if !ANDROID
	if (inputEnabled) {
		zoomValue = zoomValue + 10 * dt * theTouchInputManager.getWheel();
	}
#endif

    updateCamera(speed, dt);

	return (mode != CameraMode::None);
}

bool CameraMoveManager::moveTowardResetPositionAndZoom(float dt) {
	reset();
	zoomValue -= (zoomValue - 1) * dt;

	if (zoomValue <= 1.01)
		zoomValue = 1;
	updateCamera(-TRANSFORM(camera)->position, dt);

	return zoomValue <= 1.001 && isAtLimit(Cardinal::N);
}

void CameraMoveManager::updateCamera(const glm::vec2 & speed, float dt)
{
    auto tc = TRANSFORM(camera);
    // limit zoom value to [1, maxZoom]
    zoomValue = glm::clamp(zoomValue, 1.0f, maxZoom);
    tc->size = maxCameraSize / zoomValue;
    // constrain position within boundaries
    tc->position += speed * dt;
    tc->position.x = glm::clamp(tc->position.x,
        boundaries.swizzle(glm::X) + tc->size.x * 0.5f,
        boundaries.swizzle(glm::Z) - tc->size.x * 0.5f);
    tc->position.y = glm::clamp(tc->position.y,
        boundaries.swizzle(glm::Y) + tc->size.y * 0.5f,
        boundaries.swizzle(glm::W) - tc->size.y * 0.5f);
}
void CameraMoveManager::centerOn(const glm::vec2& position) {
	const auto* tc = TRANSFORM(camera);

	chaseTarget.x = glm::clamp(position.x,
		boundaries.swizzle(glm::X) + tc->size.x * 0.5f,
		boundaries.swizzle(glm::Z) - tc->size.x * 0.5f);
	chaseTarget.y = glm::clamp(position.y,
		boundaries.swizzle(glm::Y) + tc->size.y * 0.5f,
		boundaries.swizzle(glm::W) - tc->size.y * 0.5f);

	mode = CameraMode::Chase;
}

void CameraMoveManager::setZoom(float value) {
    LOGF_IF(value > maxZoom, "Zoom can't be greater than max!");
    LOGF_IF(value < 1.f, "Zoom can't be less than 1.0!");
    zoomValue = value;
    mode = CameraMode::Zoom;
    //should be better done than that..
    updateCamera(glm::vec2(10, 10), 1/60.);
}

bool CameraMoveManager::isAtLimit(Cardinal::Enum e, float epsilon) {
	const auto* tc = TRANSFORM(camera);
	switch (e) {
		case Cardinal::N:
			return glm::abs(tc->position.y + tc->size.y * 0.5 - boundaries.w) < epsilon;
		case Cardinal::S:
			return glm::abs(tc->position.y - tc->size.y * 0.5 - boundaries.y) < epsilon;
		default:
			LOGW("Not implemented: " << e);
			return false;
	}
}

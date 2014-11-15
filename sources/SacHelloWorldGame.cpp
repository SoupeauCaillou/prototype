#include "SacHelloWorldGame.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"
#include "systems/RenderingSystem.h"

void SacHelloWorldGame::init(const uint8_t*, int) {
    theEntityManager.CreateEntityFromTemplate("test");
}

void SacHelloWorldGame::tick(float dt) {

}

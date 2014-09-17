#include "MyTestGame.h"
#include "base/EntityManager.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"
#include "systems/RenderingSystem.h"

void MyTestGame::init(const uint8_t*, int) {
    Entity camera = theEntityManager.CreateEntityFromTemplate("camera");
    theTouchInputManager.setCamera(camera);

    theEntityManager.CreateEntityFromTemplate("test");
}

void MyTestGame::tick(float dt) {

}

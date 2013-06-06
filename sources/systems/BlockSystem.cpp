#include "BlockSystem.h"

INSTANCE_IMPL(BlockSystem);

BlockSystem::BlockSystem() : ComponentSystemImpl <BlockComponent>("Block") {
}

void BlockSystem::DoUpdate(float) {
}

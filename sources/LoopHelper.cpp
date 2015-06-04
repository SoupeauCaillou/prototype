#include "LoopHelper.h"
#include "util/Random.h"
#include "base/Log.h"

#define MAX_LOOP 32
#define MAX_PLAYER MAX_LOOP

struct Loop {
    /* ai stuff */
    struct {
        int* seeds;
        std::mt19937* generators;
        int count;
    } ai;

    struct {
        std::vector<glm::vec2>* over[MAX_LOOP]; /* mouse position */
        std::vector<uint8_t>* input[MAX_LOOP]; /* input state (down & (1 << Input::Down)) */
        std::vector<int> frames;

        int seeds[MAX_PLAYER];
        std::mt19937 generators[MAX_PLAYER];

        int count;
    } player;

    std::vector<int> unitToSaveFromDeath;
    std::vector<float> deathTime;

    float durations[MAX_LOOP];
    int id;
    int currentFrame;
};

static Loop loop;

static void seedRandomnessGenerators() {
    for (int i=0; i<loop.ai.count; i++) {
        loop.ai.generators[i].seed(loop.ai.seeds[i]);
    }

    for (int i=0; i<MAX_PLAYER; i++) {
        loop.player.generators[i].seed(loop.player.seeds[i]);
    }
}

void LoopHelper::setAICount(int count) {
    LOGV(1, "Declaring " << count << " ai units");
    loop.ai.count = count;
    loop.ai.seeds = new int[count];
    Random::N_Ints(count, loop.ai.seeds, 0, INT_MAX - 1);
    loop.ai.generators = new std::mt19937[count];
}

void LoopHelper::start() {
    loop.id = 0;
    loop.durations[0] = 0;
    loop.currentFrame = 0;

    loop.unitToSaveFromDeath.clear();
    loop.deathTime.clear();

    loop.player.count = 1;
    loop.player.over[0] = new std::vector<glm::vec2>(1024);
    loop.player.input[0] = new std::vector<uint8_t>(1024);
    loop.player.frames.resize(MAX_PLAYER);
    loop.player.frames[0] = 0;
    Random::N_Ints(MAX_PLAYER, loop.player.seeds, 0, INT_MAX - 1);

    seedRandomnessGenerators();
}

void LoopHelper::loopFailedUnitDead(int index) {
    if (loop.unitToSaveFromDeath.empty() || (index != loop.unitToSaveFromDeath.back())) {
        loop.unitToSaveFromDeath.push_back(index);
        loop.deathTime.push_back(loop.durations[loop.id]);
    }

    /* only way to increase loop count */
    if (loop.id == (loop.player.count - 1) || (index == loop.player.count - 1)) {
        loop.player.over[loop.player.count] = new std::vector<glm::vec2>(1024);
        loop.player.input[loop.player.count] = new std::vector<uint8_t>(1024);
        loop.player.frames[loop.player.count] = 0;
        loop.player.count++;
    }

    loop.id++;
    loop.durations[loop.id] = 0;
    loop.currentFrame = 0;

    seedRandomnessGenerators();
}

void LoopHelper::loopSucceeded() {
    /* take control of saved unit */
    if (!loop.unitToSaveFromDeath.empty()) {
        loop.id = loop.unitToSaveFromDeath.back();
        loop.unitToSaveFromDeath.pop_back();
        loop.deathTime.pop_back();
    }

    seedRandomnessGenerators();
}

bool LoopHelper::canFastForward(int index) {
    /* -1 because update() is called after canFastForward */
    return loop.currentFrame < loop.player.frames[index];
}

bool LoopHelper::isLoopLongerThanPrevious() {
    if (loop.id == 0) return false;
    return loop.durations[loop.id] > loop.durations[loop.id - 1];
}

float LoopHelper::loopDuration() {
    return loop.durations[loop.id];
}

void LoopHelper::update(float dt, bool fastforward) {
    loop.durations[loop.id] += dt;
    LOGV(1, "Loop #" << loop.id << " new duration = " << loop.durations[loop.id]);

    LOGV(1, "Saved input: " << LogFmt("%08x") << (int)(*loop.player.input[loop.id])[loop.currentFrame]);
    loop.currentFrame++;

    {
        auto* v = loop.player.over[loop.id];
        if ((int)v->size() <= loop.currentFrame) {
            v->resize(2 * v->size());
            loop.player.input[loop.id]->resize(v->size());
        }
    }

    if (!fastforward) {
        loop.player.frames[loop.id]++;
        /* reset input state for new frame, for active player */
        (*loop.player.input[loop.id])[loop.currentFrame] = 0;
    }
}

int LoopHelper::playerCount() {
    return loop.player.count;
}

int LoopHelper::activePlayerIndex() {
    return loop.id;
}

bool LoopHelper::input(Input::Enum i, int player) {
    LOGF_IF(player >= loop.player.count, "Requesting input for player " << player << " when there are only " << loop.player.count << " in play");
    if (loop.currentFrame >= loop.player.frames[player]) {
        return false;
    }
    return (*loop.player.input[player])[loop.currentFrame] & (1 << (int)i);
}

glm::vec2 LoopHelper::over(int player) {
    LOGF_IF(player >= loop.player.count, "Requesting over for player " << player << " when there are only " << loop.player.count << " in play");
    if (loop.currentFrame >= loop.player.frames[player]) {
        return loop.player.over[player]->back();
    }
    return (*loop.player.over[player])[loop.currentFrame];
}

void LoopHelper::save(Input::Enum i, int player) {
    LOGF_IF(player != loop.id, "Sacing input for non-active player " << loop.id);
    (*loop.player.input[player])[loop.currentFrame] |= (1 << (int)i);
}

void LoopHelper::save(glm::vec2 p, int player) {
    LOGF_IF(player != loop.id, "Sacing input for non-active player " << loop.id);
    (*loop.player.over[player])[loop.currentFrame] = p;
}

std::mt19937& LoopHelper::aiRandomGenerator(int index) {
    return loop.ai.generators[index];
}

std::mt19937& LoopHelper::playerRandomGenerator(int index) {
    return loop.player.generators[index];
}

int LoopHelper::unitToSaveFromDeath() {
    if (loop.unitToSaveFromDeath.empty())
        return -1;
    return loop.unitToSaveFromDeath.back();
}

float LoopHelper::unitDeathTime() {
    if (loop.deathTime.empty())
        return -1;
    return loop.deathTime.back();
}

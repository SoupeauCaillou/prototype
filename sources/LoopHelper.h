#pragma once

#include <random>
#include <glm/glm.hpp>

namespace Input
{
    enum Enum {
        Left_Btn = 0,
        Right_Btn,
        Up,
        Down,
        Left,
        Right,
    };

}

class LoopHelper {
    public:
        static void setAICount(int count);

        static void start();

        static void update(float dt);

        static bool isLoopLongerThanPrevious();

        static void loopFailed();

        static void loopSucceeded();

        static int playerCount();

        static int activePlayerIndex();

        static bool input(Input::Enum i, int player);

        static void save(Input::Enum i, int player);

        static void save(glm::vec2, int player);

        static glm::vec2 over(int player);

};

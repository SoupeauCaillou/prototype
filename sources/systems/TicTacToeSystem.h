#pragma once

#include "systems/System.h"

struct TicTacToeComponent {
    TicTacToeComponent() : player1(0), player2(0), currentPlayer(0), lastPlayedCell(0) {}

    Entity player1, player2, currentPlayer;
    Entity grid[81], lastPlayedCell;
};

#define theTicTacToeSystem TicTacToeSystem::GetInstance()
#define TTT(e) theTicTacToeSystem.Get(e)

UPDATABLE_SYSTEM(TicTacToe)
};

#include "ActionSystem.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/MorpionGridSystem.h"
#include "systems/TicTacToeSystem.h"


INSTANCE_IMPL(ActionSystem);

ActionSystem::ActionSystem() : ComponentSystemImpl <ActionComponent>("Action") {
    ActionComponent ac;
    componentSerializer.add(new Property<int>("action", OFFSET(action, ac), 0));
    componentSerializer.add(new EntityProperty("cell", OFFSET(SelectedCellParams.cell, ac)));
    componentSerializer.add(new EntityProperty("player", OFFSET(SelectedCellParams.player, ac)));
}

void ActionSystem::DoUpdate(float) {
    for (auto it: components) {
        const Entity e = it.first;
        auto* ac = it.second;

        switch (ac->action) {
            case EAction::None:
                break;
            case EAction::SelectedCell:
                auto * ttt = theTicTacToeSystem.getAllComponents().begin()->second;

                //change type of the cell
                MORPION_GRID(ac->SelectedCellParams.cell)->type = (ac->SelectedCellParams.player == ttt->player1) ?
                    MorpionGridComponent::Player1
                    : MorpionGridComponent::Player2;

                //update last played cell
                ttt->lastPlayedCell = ac->SelectedCellParams.cell;

                //change active player
                ttt->currentPlayer = (ac->SelectedCellParams.player == ttt->player1) ?
                    ttt->player2
                    : ttt->player1;

                //check if there is any cells that are definitively lost
                int i = MORPION_GRID(ttt->lastPlayedCell)->i;
                int j = MORPION_GRID(ttt->lastPlayedCell)->j;
                if (theMorpionGridSystem.isMiniMorpionFinished(i, j)) {
                    for (auto e : theMorpionGridSystem.getCellsForMiniMorpion(i, j, MorpionGridComponent::Available)) {
                        MORPION_GRID(e)->type = MorpionGridComponent::Lost;
                    }
                }
                break;
        }
    }
}

#pragma once

#include <vector>
#include <string>
#include <memory>

#include "../UserCommon/Board.h"
#include "../UserCommon/Tank.h"
#include "../UserCommon/Mine.h"
#include "../UserCommon/Wall.h"

//need to update to assigmnment 3

class InputParser {
public:
    explicit InputParser(const std::string& filename);

    Board getBoard() const;
    std::vector<std::unique_ptr<Tank>> getPlayer1Tanks();
    std::vector<std::unique_ptr<Tank>> getPlayer2Tanks();

    std::vector<std::unique_ptr<Mine>>& getActiveMines();
    std::vector<std::unique_ptr<Wall>>& getActiveWalls();

    int getMaxSteps() const;
    int getNumShells() const;

private:
    Board board_;
    std::vector<std::unique_ptr<Tank>> player1Tanks_;
    std::vector<std::unique_ptr<Tank>> player2Tanks_;
    std::vector<std::unique_ptr<Mine>> activeMines_;
    std::vector<std::unique_ptr<Wall>> activeWalls_;

    int maxSteps_;
    int numShells_;
};

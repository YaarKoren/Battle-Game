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

    UserCommon_207177197_301251571::Board getBoard() const;
    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Tank>> getPlayer1Tanks();
    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Tank>> getPlayer2Tanks();

    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Mine>>& getActiveMines();
    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Wall>>& getActiveWalls();

    int getMaxSteps() const;
    int getNumShells() const;

private:
    UserCommon_207177197_301251571::Board board_;
    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Tank>> player1Tanks_;
    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Tank>> player2Tanks_;
    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Mine>> activeMines_;
    std::vector<std::unique_ptr<UserCommon_207177197_301251571::Wall>> activeWalls_;

    int maxSteps_;
    int numShells_;
};

#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <unordered_map>
#include <optional>

#include "../common/TankAlgorithm.h"
#include "../common/ActionRequest.h"
#include "../UserCommon/MovingGameObject.h"
#include "../UserCommon/Position.h"
#include "../UserCommon/Direction.h"
#include "../UserCommon/Board.h"
#include "../UserCommon/MyBattleInfo.h"


namespace Algorithm_207177197_301251571 {

//Hunter Algo
class MyTankAlgorithm : public TankAlgorithm {
public:
    MyTankAlgorithm(int playerIndex, int tankIndex);

    ~MyTankAlgorithm() = default;

    ActionRequest getAction() override;
    void updateBattleInfo(BattleInfo& info) override;

    int getPlayerIndex() const { return playerIndex_; }
    int getTankIndex() const { return tankIndex_; }

private:
    int playerIndex_;
    int tankIndex_;

    std::optional<UserCommon_207177197_301251571::MyBattleInfo> currentInfo_;
    std::vector<UserCommon_207177197_301251571::Position> currentPath;
    UserCommon_207177197_301251571::Direction currentDirection_;
    int turnsSinceLastUpdate_;
    bool isDirectionInitialized_;
    static constexpr int UPDATE_INTERVAL = 4;

    std::vector<UserCommon_207177197_301251571::Position> runBFS(const UserCommon_207177197_301251571::Position& start, const UserCommon_207177197_301251571::Position& goal,
                                   const std::vector<std::vector<char>>& grid);

    UserCommon_207177197_301251571::Direction getDirectionTo(const UserCommon_207177197_301251571::Position& from, const UserCommon_207177197_301251571::Position& to) const;
    ActionRequest rotateToward(UserCommon_207177197_301251571::Direction current, UserCommon_207177197_301251571::Direction target) const;

    // Line-of-fire / obstacle helpers
    bool isEnemyInLineOfFire(const UserCommon_207177197_301251571::Position& myPos,
                                 const std::vector<std::vector<char>>& grid,
                                 char enemySymbol) const;

    bool isWallAhead(const UserCommon_207177197_301251571::Position& myPos,
                         const std::vector<std::vector<char>>& grid) const;

    // Helper to convert 4-direction to 8-direction approximation
    UserCommon_207177197_301251571::Direction approximateTo8Direction(UserCommon_207177197_301251571::Direction dir4) const;

};

}

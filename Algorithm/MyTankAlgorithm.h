#pragma once

#include <memory>

#include "../common/TankAlgorithm.h"
#include "../common/ActionRequest.h"
#include "../UserCommon/MovingGameObject.h"
#include "../UserCommon/Position.h"
#include "../UserCommon/Direction.h"
#include "../UserCommon/Board.h"

namespace Algorithm_207177197_301251571 {

class MyTankAlgorithm : public TankAlgorithm {
public:
    MyTankAlgorithm(std::unique_ptr<TankAlgorithm> actualAlgo,
                    int playerIndex, int tankIndex);

    MyTankAlgorithm(int playerIndex, int tankIndex);


    ~MyTankAlgorithm() = default;

    ActionRequest getAction() override;
    void updateBattleInfo(BattleInfo& info) override;

    int getPlayerIndex() const { return playerIndex_; }
    int getTankIndex() const { return tankIndex_; }

private:
    std::unique_ptr<TankAlgorithm> actualAlgo_;
    int playerIndex_;
    int tankIndex_;
};

}

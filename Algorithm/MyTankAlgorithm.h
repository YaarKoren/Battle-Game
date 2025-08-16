#pragma once

#include <memory>

#include "../common/TankAlgorithm.h"
#include "../common/ActionRequest.h"


namespace Algorithm_207177197_301251571 {

class MyTankAlgorithm: public TankAlgorithm {
public:
    MyTankAlgorithm(int player_index, int tank_index) {}
    ActionRequest getAction() override; //implement the algo in cpp
    void updateBattleInfo(BattleInfo&) override;

    int getPlayerIndex() const { return playerIndex_; }
    int getTankIndex() const { return tankIndex_; }


private:
    int playerIndex_;
    int tankIndex_;


};



}

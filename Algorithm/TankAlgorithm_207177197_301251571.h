#pragma once

#include "../common/TankAlgorithm.h"
#include "../common/ActionRequest.h"


namespace Algorithm_207177197_301251571 {

class TankAlgorithm_207177197_301251571 : public TankAlgorithm {
public:
    TankAlgorithm_207177197_301251571(int player_index, int tank_index) {}
    ActionRequest getAction() override {
        return ActionRequest::Shoot;
        //implement in cpp, the shoot is just an example
    }
    void updateBattleInfo(BattleInfo&) override {}
};

}

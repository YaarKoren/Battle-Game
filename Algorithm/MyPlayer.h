#pragma once

#include "../common/Player.h"
#include "../UserCommon/MyBattleInfo.h"
#include "../UserCommon/Tank.h"




namespace Algorithm_207177197_301251571 {

class MyPlayer : public Player {
public:
    MyPlayer(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells);
    ~MyPlayer() override = default;
    virtual void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) override;

private:
    int player_index_;
    size_t board_width_;
    size_t board_height_;
    size_t max_steps_;
    size_t num_shells_;

};

}

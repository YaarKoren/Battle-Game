#pragma once

#include "../common/Player.h"


namespace Algorithm_207177197_301251571 {

class Player_207177197_301251571 : public Player {
public:
    Player_207177197_301251571(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells) {}
    ~Player_207177197_301251571() override = default; //make sure it's right / needed
    virtual void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) override {}

private:
    int player_index_;
    size_t board_width_;
    size_t board_height_;
    size_t max_steps_;
    size_t num_shells_;

};

}

#include "MyPlayer.h"
#include "../common/PlayerRegistration.h"

using namespace UserCommon_207177197_301251571;

namespace Algorithm_207177197_301251571 {

REGISTER_PLAYER(MyPlayer);

MyPlayer::MyPlayer(int player_index, size_t width, size_t height, size_t max_steps, size_t num_shells)
        : player_index_(player_index),
          board_width_(width),
          board_height_(height),
          max_steps_(max_steps),
          num_shells_(num_shells) {}


void MyPlayer::updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) {
        MyBattleInfo info(satellite_view, player_index_, board_width_, board_height_, {0, 0});
        tank.updateBattleInfo(info);  // Polymorphic dispatch
}



}
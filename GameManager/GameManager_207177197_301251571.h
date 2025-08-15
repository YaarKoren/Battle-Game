#pragma once

#include "../common/AbstractGameManager.h"
#include "../UserCommon/UserCommonNamespace.h"


class GameManager_207177197_301251571 : public AbstractGameManager {
public:
    GameManager_207177197_301251571(bool verbose);  // our constructor
    ~GameManager_207177197_301251571() override;    // optional, if we need

    GameResult run(
    size_t map_width, size_t map_height,
    const SatelliteView& map, // <= a snapshot, NOT updated
    string map_name,
    size_t max_steps, size_t num_shells,
    Player& player1, string name1, Player& player2, string name2,
    TankAlgorithmFactory player1_tank_algo_factory,
    TankAlgorithmFactory player2_tank_algo_factory) override;

    bool getVerbose () const {return verbose_;}



private:
    bool verbose_;
    // you can add extra members here
};


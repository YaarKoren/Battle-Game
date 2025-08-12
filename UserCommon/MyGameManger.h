#pragma once

#include "../common/AbstractGameManager.h"

class MyGameManager : public AbstractGameManager {
public:
    MyGameManager(bool verbose);  // our constructor
    ~MyGameManager() override;    // optional, if we need

    GameResult run(
    size_t map_width, size_t map_height,
    const SatelliteView& map, // <= a snapshot, NOT updated
    string map_name,
    size_t max_steps, size_t num_shells,
    Player& player1, string name1, Player& player2, string name2,
    TankAlgorithmFactory player1_tank_algo_factory,
    TankAlgorithmFactory player2_tank_algo_factory) override;

private:
    bool verbose_;
    // you can add extra members here
};
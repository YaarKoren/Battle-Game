#pragma once

#include <string>
#include "../common/GameResult.h"

struct GMNameAndResult {
        std::string name; // own the name
        GameResult  result; // own the result (movable because of unique_ptr)
    };

struct AlgoAndScore
{
    std::string name;
    PlayerFactory player_factory;
    TankAlgorithmFactory algo_factory;
    int score;
};

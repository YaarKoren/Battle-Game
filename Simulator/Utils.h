#pragma once

#include <string>
#include "../common/GameResult.h"

#define MAX_STEPS_AFTER_SHELLS_END 40

struct GMObjectAndName {
        std::string name; // own the name
        std::unique_ptr<AbstractGameManager> GM;
    };

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

struct AlgoAndScoreSmall {
    std::string name;
    int score;
};

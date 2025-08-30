#pragma once

#include <string>
#include <atomic>

#include "../common/GameResult.h"
#include "../common/AbstractGameManager.h"
#include "../common/PlayerRegistration.h"
#include "../common/TankAlgorithmRegistration.h"

#define MAX_STEPS_AFTER_SHELLS_END 40

//-----------------------------comparative--------------------------------------------------

struct GMObjectAndName {
        std::string name; // own the name
        std::unique_ptr<AbstractGameManager> GM;
    };

struct GMNameAndResult {
    std::string name; // own the name
    GameResult  result; // own the result (movable because of unique_ptr)
};


//Multi Threading helper struct
struct GMTask {
    size_t gm_index;
    size_t result_slot;
};

//Multi Threading helper struct
struct GMProducer {
    const std::vector<GMTask>* tasks;                 // not owning
    const std::vector<GMObjectAndName>* GMs;          // not owning
    std::vector<GMNameAndResult>* results;            // not owning
    Player* player1;                                   // not owning
    Player* player2;                                   // not owning
    TankAlgorithmFactory p1_factory;                   // copies are fine
    TankAlgorithmFactory p2_factory;                   // copies are fine
    const std::unique_ptr<SatelliteView>* map;         // pointer to your unique_ptr
    std::string map_name;
    size_t map_width, map_height, max_steps, num_shells;
    std::string player_1_name, player_2_name;                      // if you have these names in scope

    std::atomic_size_t next{0};                        // work index

    std::optional<std::function<void()>> getTask() {
        size_t i = next.fetch_add(1, std::memory_order_relaxed);
        if (i >= tasks->size()) return std::nullopt;

        GMTask t = (*tasks)[i];

        // capture everything this unit of work needs by value (or safe pointer)
        return std::function<void()>([=, this]() {
            try {
                // Run this GM exactly once (each task is a distinct GM)
                auto& gm_and_name = (*GMs)[t.gm_index];
                auto& gm = *gm_and_name.GM;

                GameResult game_result = gm.run(
                    map_width, map_height, *(*map), map_name,
                    max_steps, num_shells,
                    *player1, player_1_name,
                    *player2, player_2_name,
                    p1_factory, p2_factory
                );

                // Store result in its unique slot (no lock)
                (*results)[t.result_slot].result = std::move(game_result);
            } catch (const std::exception& e) {
                // Record a failure in-place (define your policy)
                GameResult fail{};
                fail.winner = 0;
                fail.reason = GameResult::MAX_STEPS; // or a custom "ERROR" if you add one
                fail.rounds = 0;
                (*results)[t.result_slot].result = std::move(fail);
                // Optionally: log e.what()
            }
        });
    }
};


//------------------------------competition-----------------------------------------------
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



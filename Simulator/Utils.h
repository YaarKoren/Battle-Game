#pragma once

#include <string>
#include <atomic>
#include <optional>
#include <functional>

#include "../common/GameResult.h"
#include "../common/AbstractGameManager.h"
#include "../common/PlayerRegistration.h"
#include "../common/TankAlgorithmRegistration.h"

#include "MapParser.h"

#define MAX_STEPS_AFTER_SHELLS_END 40

//----------------------------------------comparative--------------------------------------------------

struct GMObjectAndName {
        std::string name; // own the name
        std::unique_ptr<AbstractGameManager> GM;
    };

struct GMNameAndResult {
   std::string name; // own the name
   GameResult  result; // own the result (movable because of unique_ptr)
};


//Multi Threading helper structs

struct GMTask {
    size_t gm_index;
    size_t result_slot;
};


struct GMProducer {
    const std::vector<GMTask>* tasks;                 // not owning
    const std::vector<GMObjectAndName>* GMs;          // not owning
    std::vector<GMNameAndResult>* results;            // not owning
    Player* player1;                                   // not owning
    Player* player2;                                   // not owning
    TankAlgorithmFactory p1_algo_factory;                   // copies are fine
    TankAlgorithmFactory p2_algo_factory;                   // copies are fine
    const std::unique_ptr<SatelliteView>* map;
    std::string map_name;
    size_t map_width, map_height, max_steps, num_shells;
    std::string player_1_name, player_2_name;

    std::atomic_size_t next{0};                        // work index

    std::optional<std::function<void()>> getTask() {
        size_t i = next.fetch_add(1, std::memory_order_relaxed);
        if (i >= tasks->size()) return std::nullopt;

        GMTask t = (*tasks)[i];

        return std::function<void()>([=, this]() { //task lambda
            try {
                // run this GM exactly once (each task is a distinct GM)
                auto& gm_and_name = (*GMs)[t.gm_index];
                auto& gm = *gm_and_name.GM;

                GameResult game_result = gm.run(
                    map_width, map_height, *(*map), map_name,
                    max_steps, num_shells,
                    *player1, player_1_name,
                    *player2, player_2_name,
                    p1_algo_factory, p2_algo_factory
                );

                // Store result in its unique slot (no lock)
                (*results)[t.result_slot].result = std::move(game_result);
            } catch (const std::exception& e) {
                //(*results)[t.result_slot].result.reset();               // mark as error; After this, [t.result_slot].result.has_value() will be false (commented it out cuz it caused problems we have no time to fix)
                std::cout << "[SIM] Error during run(): " <<  e.what() << "\n";
            } catch (...) {
                //(*results)[t.result_slot].result.reset();
                std::cout << "[SIM] Error during run(): Unknown\n";
            }
        });
    }
};


//------------------------------------------competition-----------------------------------------------
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

//Multi Threading helper structs

struct CompetitionTask {
    size_t map_idx;  // k
    size_t i;        // algo index
    size_t j;        // opponent index
    size_t slot;     // index into results[]
};

struct ScoreDelta {
    // how this single game changes the scores of i and j
    size_t i, j;
    int di, dj;
};

struct GMCompetitionProducer {
    const std::vector<CompetitionTask>* tasks;
    std::vector<ScoreDelta>* deltas;
    bool verbose;

    // read-only context:
    const std::vector<MapParser::MapArgs>* maps_data;
    const std::vector<AlgoAndScore>* algos_and_scores; // to get factories and names
    GameManagerFactory gm_factory;                      // create fresh GM per task

    std::atomic_size_t next{0};

    std::optional<std::function<void()>> getTask() {
        size_t idx = next.fetch_add(1, std::memory_order_relaxed);
        if (idx >= tasks->size()) return std::nullopt;

        CompetitionTask t = (*tasks)[idx];

        // capture by value pointers/indices; no shared mutation inside
        return std::function<void()>([=, this]() { //task lambda
            try
            {
                const auto& mapArgs = (*maps_data)[t.map_idx];

                // create fresh instances (cheap & thread-safe)
                auto gm = gm_factory(verbose);

                // Players for i and j
               std::unique_ptr<Player> Player1 = (*algos_and_scores)[t.i].player_factory(
                   /*player index*/ 1, mapArgs.map_width_, mapArgs.map_height_, mapArgs.max_steps_, mapArgs.num_shells_);
               std::unique_ptr<Player> Player2 = (*algos_and_scores)[t.j].player_factory(
                   /*player index*/ 2, mapArgs.map_width_, mapArgs.map_height_, mapArgs.max_steps_, mapArgs.num_shells_);

                // algo factories for  i and j
                auto p1_algo_factory = (*algos_and_scores)[t.i].algo_factory;
                auto p2_algo_factory = (*algos_and_scores)[t.j].algo_factory;

                // names
                std::string name1 = (*algos_and_scores)[t.i].name;
                std::string name2 = (*algos_and_scores)[t.j].name;

                // Run one game
                GameResult gr = gm->run(
                    mapArgs.map_width_, mapArgs.map_height_, *mapArgs.map_, mapArgs.map_name_,
                    mapArgs.max_steps_, mapArgs.num_shells_,
                    *Player1, name1,
                    *Player2 , name2,
                    p1_algo_factory, p2_algo_factory
                );

                // compute score delta for this single game
                ScoreDelta d{t.i, t.j, 0, 0};
                if      (gr.winner == 1) { d.di += 3;  }
                else if (gr.winner == 2) { d.dj += 3; }
                else { d.di += 1; d.dj += 1; } // tie

                // store in unique slot (no lock)
                (*deltas)[t.slot] = d;
            } catch (const std::exception& e)
            {
                 (*deltas)[t.slot] = ScoreDelta{t.i, t.j, 0, 0}; // no points
            } catch (...)
            {
                (*deltas)[t.slot] = ScoreDelta{t.i, t.j, 0, 0}; // no points
            }
        });
    }
};

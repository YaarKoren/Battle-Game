#pragma once

#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <chrono>
#include <cmath>
#include <iomanip>   // <-- for setw, setfill
#include <filesystem>


#include "../common/SatelliteView.h"
#include "Utils.h"



class GameResultPrinter {
  public:
    GameResultPrinter() = default;

    virtual ~GameResultPrinter() = default;
    void printCompetitionResults(const std::vector<AlgoAndScoreSmall>& algos_and_scores,
                                    const std::string& maps_folder_path,
                                    const std::string& game_manager_clean_name,
                                    const std::string& algos_folder_path);

    static void printComparativeResults(std::vector<GMNameAndResult> results,
                                        std::string folder_path,
                                        size_t map_width, size_t map_height,
                                        std::string game_map_filename,
                                        std::string algo1_so_filename,
                                        std::string algo2_so_filename,
                                        size_t max_steps);

    static void printCompetitionResults();

  private:

    //time helper function
    static std::string makeUniquePath(std::string folder_path, std::string mode_name);

    //Comparative mode - helper functions and structs
    struct ResultKey {
        int winner;
        GameResult::Reason reason;
        size_t rounds;
        std::string final_snapshot;

        bool operator==(const ResultKey& o) const {
            return winner == o.winner &&
                   reason == o.reason &&
                   rounds == o.rounds &&
                   final_snapshot == o.final_snapshot;
        }
    };

    struct ResultKeyLess {
        bool operator()(ResultKey const& a, ResultKey const& b) const //takes two ResultKey objects and returns true if a < b
      {
            return std::tie(a.winner, a.reason, a.rounds, a.final_snapshot) <
                   std::tie(b.winner, b.reason, b.rounds, b.final_snapshot);
        }
    };

    struct Group {
        ResultKey key;
        std::vector<const GMNameAndResult*> members;
    };


    static ResultKey makeKey(const GameResult& r,
                             size_t map_width, size_t map_height);
    static std::string renderView(const SatelliteView& sv,
                                  size_t map_width, size_t map_height);
    static std::string blankSnapshot(size_t map_width, size_t map_height);
    static std::string reasonToString(GameResult::Reason r);
    static std::string resultMessage(const GameResult& r, size_t max_steps);
    static size_t getNumberOfTanks(const GameResult& r, int player);



    //Competition Mode - helper functions



};



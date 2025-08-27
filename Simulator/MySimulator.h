#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstddef>
#include <unordered_map>
#include <cstdlib>
#include <optional>
#include <filesystem>
#include <utility>

#include "../common/ActionRequest.h"
#include "../common/BattleInfo.h"
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "../common/AbstractGameManager.h"
#include "../common/GameResult.h"
#include "../common/SatelliteView.h"

#include "AlgorithmRegistrar.h"
#include "GameManagerRegistrar.h"
#include "SharedLib.h"              // cross-platform dlopen/LoadLibrary wrapper

#include "Mode.h"
#include "CmdArgsParser.h"
#include "MapParser.h"
#include "Utils.h"
#include "GameResultPrinter.h"


class MySimulator {
public:
    explicit MySimulator(CmdArgsParser::CmdArgs args);
    ~MySimulator() = default;

    int run();                     // decides comparative vs competitive


private:
    CmdArgsParser::CmdArgs args_;

    //destructure args_ for convenience
    Mode mode_;
    bool verbose_;
    int threads_num_;
    //comparative mode
    std::string mapPath;
    std::string managersFolder;
    std::string algo1SO;
    std::string algo2SO;
    //competition mode
    std::string mapsFolder;
    std::string managerPath;
    std::string algosFolder;

    // Keep .so handles alive for the entire match (RAII)
    std::vector<std::unique_ptr<SharedLib>> algo_libs_; // TankAlgo+Player .so handles
    std::vector<std::unique_ptr<SharedLib>> GM_libs_; // Game Manager .so handles


    void runComparative(std::ostringstream& oss);
    void runCompetitive(std::ostringstream& oss);

    //general helper functions
    void parse_map(std::ostringstream& oss, std::string& map_name, size_t& map_width, size_t& map_height, size_t& max_steps, size_t& num_shells,
                   std::unique_ptr<SatelliteView>& map) const;


    //general helper functions - so files loading
    static std::string getCleanFileName(const std::string& path);
    size_t loadAlgoAndPlayerAndGetIndex(const std::string& so_path);
    size_t loadGameManagerAndGetIndex(const std::string& so_path);
    static std::vector<std::string> getSoFilesList(const std::string& dir_path);



    //comparative mode helper functions - so files loading
    void load_and_validate_comparative(std::unique_ptr<Player>& player1, std::unique_ptr<Player>& player2,
                                           TankAlgorithmFactory& p1_algo_factory, TankAlgorithmFactory& p2_algo_factory,
                                           size_t map_width, size_t map_height, size_t max_steps, size_t num_shells);

    void load_and_validate_comparative(std::ostringstream& oss, std::vector<GMObjectAndName>& GMs);



    //competition mode  helper functions
    void load_and_validate_competition(std::unique_ptr<AbstractGameManager>& GM);
    void load_and_validate_competition(std::ostringstream& oss, std::vector<AlgoAndScore>& algos_and_scores);
    void read_maps(std::ostringstream& oss, std::vector<MapParser::MapArgs>& maps_data) const;

    static std::vector<std::string> getFilesList(const std::string& dir_path);
    static int getOpponentIdx(size_t l, size_t k, size_t N);
    static void runGameAndKeepScore(size_t l,  int opp, std::vector<AlgoAndScore>& algos_and_scores,
        size_t map_width,  size_t map_height, size_t max_steps, size_t num_shells,
        const std::string& map_name,
        const std::unique_ptr<SatelliteView>& map,
        const std::unique_ptr<AbstractGameManager>& GM);



};


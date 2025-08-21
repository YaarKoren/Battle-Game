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
    void runComparative();          // non-static, uses args_
    void runCompetitive();          // non-static, uses args_
    CmdArgsParser::CmdArgs args_;

    //so files loading - helper functions
    static std::string getCleanFileName(const std::string& path);
    static size_t loadAlgoAndPlayerAndGetIndex(const std::string& so_path,
        std::vector<std::unique_ptr<SharedLib>>& open_libs);
    static size_t loadGameManagerAndGetIndex(const std::string& so_path,
        std::vector<std::unique_ptr<SharedLib>>& open_libs);
    static std::vector<std::string> getSoFilesList(const std::string& dir_path);


    //competition mode - helper functions
    static std::vector<std::string> getFilesList(const std::string& dir_path);
    static int MySimulator::getOpponentIdx(int l, int k, size_t N);
    static void runGameAndKeepScore(int l,  int opp, std::vector<AlgoAndScore> algos_and_scores,
        size_t map_width,  size_t map_height, size_t max_steps, size_t num_shells,
        const std::string& map_name,
        const std::unique_ptr<SatelliteView>& map,
        const std::unique_ptr<AbstractGameManager>& GM);



};


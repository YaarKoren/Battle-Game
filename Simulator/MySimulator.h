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

#include "../common/ActionRequest.h"
#include "../common/BattleInfo.h"
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "../common/AbstractGameManager.h"
#include "../common/GameResult.h"

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

    void run();                     // decides comparative vs competitive


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
    static std::vector<size_t> loadTankAlgosAndPlayersFromDir(const std::string& dir,
        std::vector<std::unique_ptr<SharedLib>>& open_libs);
    static std::vector<size_t> loadGMFromDir(const std::string& dir,
    std::vector<std::unique_ptr<SharedLib>>& open_libs);
};


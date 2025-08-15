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

#include "../common/ActionRequest.h"
#include "../common/BattleInfo.h"
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "../common/AbstractGameManager.h"


#include "AlgorithmRegistrar.h"
#include "GameManagerRegistrar.h"
//#include "SharedLibrary.h"              // cross-platform dlopen/LoadLibrary wrapper

#include "Mode.h"
#include "CmdArgsParser.h"


class Simulator {
public:
    explicit Simulator(CmdArgsParser::CmdArgs args);
    ~Simulator() = default;

    int run();                     // decides comparative vs competitive

private:
    int runComparative();          // non-static, uses args_
    int runCompetitive();          // non-static, uses args_
    CmdArgsParser::CmdArgs args_;
};


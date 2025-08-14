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



